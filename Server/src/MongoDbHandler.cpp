#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include "logging/Logger.h"
#include "MongoDbHandler.h"
#include "Util.h"

bool MongoDbHandler::createUser(const std::string& username, const std::string& password)
{
    // Create user document
    SERVER_INFO("MongoDbHandle::registerUser");
    try
    {
        // Generate salt and hash password
        std::string salt = generateSalt(16);
        std::string hashedPassword = hashPassword(password, salt);

        // Initialize empty server array
        bsoncxx::builder::basic::array serversArr = bsoncxx::builder::basic::array{};

        // Define document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "username"      << username
            << "password_hash" << hashedPassword
            << "salt"          << salt
            << "servers"       << serversArr
            << "created_at"    << std::to_string(getSecondsSinceEpoch())
            << "last_login"    << 0
            << "status"        << (int)UserStatus::OFFLINE
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        auto creationResult = insertOneWithRetry(m_userCollection, newDoc.view());
        if (!creationResult)
            SERVER_INFO("User document could not be created");

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteUser(const std::string& userId)
{
    SERVER_INFO("MongoDbHandle::deleteUser");
    try
    {
        // Delete user and retrieve document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(userId)
            << bsoncxx::builder::stream::finalize;

        auto result = findOneAndDeleteWithRetry(m_userCollection, filter.view());
        if (!result)
            SERVER_ERROR("Could not find user id.");

        auto view = result->view();

        // Go through user's server list and remove them from each server
        if (!view["servers"] || view["servers"].type() != bsoncxx::type::k_array)
            SERVER_ERROR("Couldn't find servers array on user document");

        std::vector<std::string> serverIds;
        for (const auto& elem : view["servers"].get_array().value)
            if (elem.type() == bsoncxx::type::k_utf8)
                serverIds.push_back(std::string(elem.get_string().value));

        for (std::string serverId : serverIds)
            if (!addRemoveMemberFromServer(serverId, userId, "$pull"))
                SERVER_ERROR("Failed to remove member from server");

        // Go through user's ownedServer list and delete them all
        if (!view["owned_servers"] || view["owned_servers"].type() != bsoncxx::type::k_array)
            SERVER_ERROR("Couldn't find owned_servers array on user document");

        std::vector<std::string> ownedServerIds;
        for (const auto& elem : view["owned_servers"].get_array().value)
            if (elem.type() == bsoncxx::type::k_utf8)
                ownedServerIds.push_back(std::string(elem.get_string().value));
        
        for(std::string ownedServerId : ownedServerIds)
            if(!deleteServer(ownedServerId))
                SERVER_ERROR("Failed to delete owned server");

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::login(const std::string& username, const std::string& password)
{
    SERVER_INFO("MongoDbHandle::login");
    try
    {
        // Define the query document to find the user
        auto findFilter = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Perform the find_one operation to check if the username exists
        auto findResult = findOneWithRetry(m_userCollection, findFilter.view());
        if (!findResult)
            SERVER_ERROR("Username does not exist.");

        // Get salt from the returned document
        auto doc = *findResult;
        auto salt = doc["salt"].get_string();

        // Hash given password with salt see if it matches password_hash in retrieved document
        if (doc["password_hash"].get_string().value != hashPassword(password, std::string(salt.value)))
            SERVER_ERROR("Incorrect Password");

        // Define the filter to find the document to update
        auto updateFilter = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Define the update to update last_login and status
        auto update = bsoncxx::builder::stream::document{}
            << "$set"
            << bsoncxx::builder::stream::open_document
            << "last_login" << std::to_string(getSecondsSinceEpoch())
            << "status" << (int)UserStatus::ONLINE
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Update user last_login and status
        auto updateResult = updateOneWithRetry(m_userCollection, updateFilter.view(), update.view());
        if (!updateResult)
            SERVER_ERROR("No documents matched the query.");
        
        SERVER_INFO("Document updated successfully.");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::logout(const std::string& userId)
{
    // Update user document status
    SERVER_INFO("MongoDbHandle::logout");
    try
    {
        // Define the filter to find the document to update
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(userId)
            << bsoncxx::builder::stream::finalize;

        // Define the update to update last_login and status
        auto update = bsoncxx::builder::stream::document{}
            << "$set"
            << bsoncxx::builder::stream::open_document
            << "status" << (int)UserStatus::OFFLINE
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform the update operation
        auto updateResult = updateOneWithRetry(m_userCollection, filter.view(), update.view());
        if (!updateResult)
            SERVER_ERROR("No documents matched the query.");
        
        SERVER_INFO("Successfully logged out.");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createServer(const std::string& serverName, const std::string& userId)
{
    SERVER_INFO("MongoDbHandle::createServer");

    std::string serverId;
    std::string channelId;

    if (!createServerDoc(serverName, userId, serverId))
        SERVER_ERROR("Server document not created");

    if (!createChannelDoc(serverId, "Home", channelId))
        SERVER_ERROR("Channel document not created");

    if (!addRemoveOwnedServerFromUser(serverId, userId, "$push"))
        SERVER_ERROR("Server id not added to user owned server list");

    if (!addRemoveChannelFromServer(serverId, channelId, "$push"))
        SERVER_ERROR("Server id not added to user server list");

    SERVER_INFO("Successfully created server");
    return true;
}

bool MongoDbHandler::deleteServer(const std::string& serverId)
{
    SERVER_INFO("MongoDbHandle::deleteServer");
    
    std::vector<std::string> channelIds;
    std::vector<std::string> memberIds;

    if (!deleteServerDoc(serverId, channelIds, memberIds))
        SERVER_ERROR("Server document not deleted");

    if (!deleteChannelDocs(serverId))
        SERVER_ERROR("Channel documents not deleted");

    // Do this better. Batch delete all docs hopefully
    for(std::string channelId : channelIds)
        if (!deleteChannelMessageDocs(channelId))
            SERVER_ERROR("Channel message documents not deleted");

    if (!removeServerFromAllMembers(memberIds, serverId))
        SERVER_ERROR("Server id not removed from users");

    SERVER_INFO("Successfully deleted server");
    return true;
}

bool MongoDbHandler::joinServer(const std::string& serverId, const std::string& userId)
{
    SERVER_INFO("MongoDbHandle::joinServer");
    if(!addRemoveMemberFromServer(serverId, userId, "$push"))
        SERVER_ERROR("User not added to server member list");

    if (!addRemoveServerFromUser(userId, serverId, "$push"))
        SERVER_ERROR("Server not added to user server list");

    SERVER_INFO("Successfully joined server");
    return true;
}

bool MongoDbHandler::leaveServer(const std::string& serverId, const std::string& userId)
{
    SERVER_INFO("MongoDbHandle::leaveServer");

    if(!addRemoveMemberFromServer(serverId, userId, "$pull"))
        SERVER_ERROR("User not removed from server member list");

    if (!addRemoveServerFromUser(userId, serverId, "$pull"))
        SERVER_ERROR("Server not removed from user server list");

    SERVER_INFO("Successfully left server");
    return true;
}

bool MongoDbHandler::createChannel(const std::string& serverId, const std::string& channelName)
{
    SERVER_INFO("MongoDbHandle::createChannel");

    std::string channelId;
    if (!createChannelDoc(serverId, channelName, channelId))
        SERVER_ERROR("Channel doc not created");

    if (!addRemoveChannelFromServer(serverId, channelId, "$push"))
        SERVER_ERROR("Channel not added to server channel list");

    SERVER_INFO("Successfully created channel");
    return true;
}

bool MongoDbHandler::deleteChannel(const std::string& serverId, const std::string& channelId)
{
    SERVER_INFO("MongoDbHandle::deleteChannel");

    if (!deleteChannelMessageDocs(channelId))
        SERVER_ERROR("Channel messages not deleted");

    if(!deleteChannelDoc(channelId))
        SERVER_ERROR("Channel doc not deleted");

    if (!addRemoveChannelFromServer(serverId, channelId, "$pull"))
        SERVER_ERROR("Channel not removed from server channel list");

    SERVER_INFO("Successfully deleted channel");
    return true;
}

bool MongoDbHandler::sendMessage(const std::string& userId, const std::string& channelId, const std::string& content)
{
    SERVER_INFO("MongoDbHandle::sendMessage");
    
    std::string messageId;
    if(!createMessageDoc(channelId, userId, content, messageId))
        SERVER_ERROR("Message doc not created");
    
    if (!addRemoveMessageFromChannel(channelId, messageId, "$push"))
        SERVER_ERROR("Message not added to channel message list");

    SERVER_INFO("Successfully created message");
    return true;
}

bool MongoDbHandler::deleteMessage(const std::string& channelId, const std::string& messageId)
{
    SERVER_INFO("MongoDbHandle::deleteMessage");

    if (!deleteMessageDoc(messageId))
        SERVER_ERROR("Message doc not deleted");
    
    if (!addRemoveMessageFromChannel(channelId, messageId, "$pull"))
        SERVER_ERROR("Message not removed from channel message list");
    
    SERVER_INFO("Successfully deleted message");
    return true;
}

bool MongoDbHandler::editMessage(const std::string& messageId, const std::string& content)
{
    SERVER_INFO("MongoDbHandle::editMessage");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(messageId)
            << bsoncxx::builder::stream::finalize;
        
        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << "$set"
            << bsoncxx::builder::stream::open_document
            << "content" << content
            << "edited_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!updateOneWithRetry(m_messageCollection, filter.view(), update.view()))
            SERVER_INFO("Message document could not be edited");

        SERVER_INFO("Successfully edited message");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::getServerChannels()
{
    return true;
}

bool MongoDbHandler::getServerMembers()
{
    return true;
}

bool MongoDbHandler::getChannelMessages()
{
    return true;
}

bool MongoDbHandler::createServerDoc(const std::string& serverName, const std::string& userId, std::string& serverId)
{
    SERVER_INFO("MongoDbHandle::createServerDoc");
    try
    {
        std::string defaultChannelName = "Home";

        // Initialize empty members array
        bsoncxx::builder::basic::array membersArr = bsoncxx::builder::basic::array{};
        membersArr.append(bsoncxx::oid(userId));

        // Initialize channels array
        bsoncxx::builder::basic::array channelsArr = bsoncxx::builder::basic::array{};
        channelsArr.append();

        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "name"       << serverName
            << "owner_id"   << bsoncxx::oid(userId)
            << "members"    << membersArr
            << "channels"   << channelsArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        auto result = insertOneWithRetry(m_serverCollection, newDoc.view());
        if (!result)
            SERVER_INFO("Failed to create server doc.");

        SERVER_INFO("Successfully created server document");
        serverId = result->inserted_id().get_oid().value.to_string();
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteServerDoc(const std::string& serverId, std::vector<std::string>& channelIds, std::vector<std::string>& memberIds)
{
    SERVER_INFO("MongoDbHandle::deleteServerDoc");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        auto result = findOneAndDeleteWithRetry(m_serverCollection, filter.view());
        if (!result)
            SERVER_INFO("Failed to delete server doc.");

        SERVER_INFO("Successfully deleted server document");

        auto view = result->view();
        if (!view["channels"] || view["channels"].type() != bsoncxx::type::k_array)
            SERVER_ERROR("Couldn't find channels array on server document");

        for (const auto& elem : view["channels"].get_array().value)
            if (elem.type() == bsoncxx::type::k_utf8)
                channelIds.push_back(std::string(elem.get_string().value));

        if (!view["members"] || view["members"].type() != bsoncxx::type::k_array)
            SERVER_ERROR("Couldn't find members array on server document");

        for (const auto& elem : view["members"].get_array().value)
            if (elem.type() == bsoncxx::type::k_utf8)
                memberIds.push_back(std::string(elem.get_string().value));

        SERVER_INFO("Successfully deleted server document");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createChannelDoc(const std::string& serverId, const std::string& channelName, std::string& channelId)
{
    SERVER_INFO("MongoDbHandle::createChannelDoc");
    try
    {
        // Initialize empty messages array
        bsoncxx::builder::basic::array messagesArr = bsoncxx::builder::basic::array{};
        messagesArr.append();

        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "server_id" << bsoncxx::oid(serverId)
            << "name" << channelName
            << "messages" << messagesArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion

        insertOneResult result = insertOneWithRetry(m_channelCollection, newDoc.view());
        if (!result)
            SERVER_INFO("Failed to create channel doc.");

        SERVER_INFO("Successfully created channel document");
        channelId = result->inserted_id().get_oid().value.to_string();
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteChannelDoc(const std::string& channelId) 
{
    SERVER_INFO("MongoDbHandle::deleteChannelDoc");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(channelId)
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteOneWithRetry(m_channelCollection, filter.view()))
            SERVER_INFO("Failed to delete channel doc.");

        SERVER_INFO("Successfully deleted channel document");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteChannelDocs(const std::string& serverId)
{
    SERVER_INFO("MongoDbHandle::deleteChannelDoc");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "server_id" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteManyWithRetry(m_channelCollection, filter.view()))
            SERVER_INFO("Failed to delete channel docs.");

        SERVER_INFO("Successfully deleted channel documents");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createMessageDoc(const std::string& channelId, const std::string& userId, const std::string& content, std::string& messageId)
{
    SERVER_INFO("MongoDbHandle::createMessageDoc");
    try
    {
        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "channel_id" << bsoncxx::oid(channelId)
            << "user_id" << bsoncxx::oid(userId)
            << "content" << content
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        insertOneResult result = insertOneWithRetry(m_messageCollection, newDoc.view());
        if (!result)
            SERVER_INFO("Failed to create message doc.");

        SERVER_INFO("Successfully created message document");
        messageId = result->inserted_id().get_oid().value.to_string();
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteMessageDoc(const std::string& messageId)
{
    SERVER_INFO("MongoDbHandle::deleteMessageDoc");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(messageId)
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteOneWithRetry(m_messageCollection, filter.view()))
            SERVER_INFO("Failed to delete message doc.");

        SERVER_INFO("Successfully deleted message document");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteChannelMessageDocs(const std::string& channelId)
{
    SERVER_INFO("MongoDbHandle::deleteChannelMessageDocs");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "channel_id" << bsoncxx::oid(channelId)
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteManyWithRetry(m_messageCollection, filter.view()))
            SERVER_INFO("Failed to delete message docs.");

        SERVER_INFO("Successfully deleted message documents");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::removeServerFromAllMembers(const std::vector<std::string>& members, const std::string& serverId)
{
    SERVER_INFO("MongoDbHandle::removeServerFromAllMembers");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "servers"
            << bsoncxx::builder::stream::open_document
            << "$elemMatch" << bsoncxx::types::b_string(serverId)
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << "$pull"
            << bsoncxx::builder::stream::open_document
            << "servers" << serverId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform updates
        if (!updateManyWithRetry(m_userCollection, filter.view(), update.view()))
            SERVER_INFO("Failed to remove server from members.");

        SERVER_INFO("Successfully removed server from members");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::removeUserFromAllServers()
{
    return true;
}

bool MongoDbHandler::addRemoveMemberFromServer(const std::string& serverId, const std::string& userId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveMemberFromServer");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "members" << userId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_serverCollection, filter.view(), update.view()))
            SERVER_INFO("No documents matched the filter");

        SERVER_INFO("Member successfully {} server", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveServerFromUser(const std::string& serverId, const std::string& userId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveServerFromUser");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(userId)
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "servers" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_userCollection, filter.view(), update.view()))
            SERVER_INFO("No documents matched the filter");

        SERVER_INFO("Server successfully {} user", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveOwnedServerFromUser(const std::string& serverId, const std::string& userId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveOwnedServerFromUser");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(userId)
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "servers" << bsoncxx::oid(serverId)
            << "owned_servers" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_userCollection, filter.view(), update.view()))
            SERVER_INFO("No documents matched the filter");

        SERVER_INFO("Server successfully {} user", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveChannelFromServer(const std::string& serverId, const std::string& channelId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addChannelToServer");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(serverId)
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "channels" << bsoncxx::oid(channelId)
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_serverCollection, filter.view(), update.view()))
            SERVER_INFO("No documents matched the filter");

        SERVER_INFO("Channel successfully {} server", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveMessageFromChannel(const std::string& channelId, const std::string& messageId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveMessageFromChannel");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << bsoncxx::oid(channelId)
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "messages" << bsoncxx::oid(messageId)
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_channelCollection, filter.view(), update.view()))
            SERVER_INFO("No documents matched the filter");

        SERVER_INFO("Message successfully {} channel", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}




//bool MongoDbHandler::deleteChannels(std::string serverName)
//{
//    SERVER_INFO("MongoDbHandle::deleteChannel");
//    try
//    {
//        // Prepare filter document
//        auto filter = bsoncxx::builder::stream::document{}
//            << "server" << serverName
//            << bsoncxx::builder::stream::finalize;
//
//        // Perform insertion
//        if (!deleteManyWithRetry(m_channelCollection, filter.view()))
//        {
//            SERVER_INFO("Failed to delete channels.");
//            return false;
//        }
//
//        SERVER_INFO("Channels successfully deleted");
//        return true;
//    }
//    catch (std::exception& e)
//    {
//        SERVER_ERROR("{}", e.what());
//        return false;
//    }
//}




findOneResult MongoDbHandler::findOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                               int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::findOneWithRetry");
    int attempt = 0;
    findOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find_one(filter);
            if (result)
                SERVER_INFO("Document found successfully.");
            else
                SERVER_ERROR("Document not found.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Find attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

findManyResult MongoDbHandler::findManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                 int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::findManyWithRetry");
    int attempt = 0;
    findManyResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find(filter);
            if (result)
                SERVER_INFO("Documents found successfully.");
            else
                SERVER_ERROR("Documents not found.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Find attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

insertOneResult MongoDbHandler::insertOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& document,
                                                   int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::insertOneWithRetry");
    int attempt = 0;
    insertOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.insert_one(document);
            if (result)
                SERVER_INFO("Document inserted successfully.");
            else
                SERVER_ERROR("Document not inserted.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Insert attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

insertManyResult MongoDbHandler::insertManyWithRetry(mongocxx::collection& collection, const std::vector<bsoncxx::document::view>& documents,
                                                     int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::insertManyWithRetry");
    int attempt = 0;
    insertManyResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.insert_many(documents);
            if (result)
                SERVER_INFO("Documents inserted successfully.");
            else
                SERVER_ERROR("Documents not inserted.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Insert attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

updateResult MongoDbHandler::updateOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::updateOneWithRetry");
    int attempt = 0;
    updateResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.update_one(filter, update);
            if (result)
                SERVER_INFO("Document updated successfully.");
            else
                SERVER_ERROR("Document not updated.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Update attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

findOneResult MongoDbHandler::findOneAndUpdateWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                        int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::findOneAndUpdateWithRetry");
    int attempt = 0;
    findOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find_one_and_update(filter, update);
            if (result)
                SERVER_INFO("Document updated successfully.");
            else
                SERVER_ERROR("Document not updated.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Update attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}


updateResult MongoDbHandler::updateManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                 int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::updateManyWithRetry");
    int attempt = 0;
    updateResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.update_many(filter, update);
            if (result)
                SERVER_INFO("Documents updated successfully.");
            else
                SERVER_ERROR("Documents not updated.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Update attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

deleteResult MongoDbHandler::deleteOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::deleteOneWithRetry");
    int attempt = 0;
    deleteResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.delete_one(filter);
            if (result)
                SERVER_INFO("Document deleted successfully.");
            else
                SERVER_ERROR("Document not deleted.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Delete attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

deleteResult MongoDbHandler::deleteManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                 int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::deleteManyWithRetry");
    int attempt = 0;
    deleteResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.delete_many(filter);
            if (result)
                SERVER_INFO("Documents deleted successfully.");
            else
                SERVER_ERROR("Documents not deleted.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Delete attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}

findOneResult MongoDbHandler::findOneAndDeleteWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                       int max_retries, int retry_interval_ms)
{
    SERVER_INFO("MongoDbHandle::findOneAndDeleteWithRetry");
    int attempt = 0;
    findOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find_one_and_delete(filter);
            if (result)
                SERVER_INFO("Document deleted successfully.");
            else
                SERVER_ERROR("Document not deleted.");
            break;
        }
        catch (const std::exception& e)
        {
            SERVER_ERROR("Delete attempt {} failed: {}", (attempt + 1), e.what());
            if (attempt + 1 >= max_retries)
            {
                SERVER_ERROR("Maximum retries reached. Giving up.");
                throw e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
        }
        attempt++;
    }
    return result;
}