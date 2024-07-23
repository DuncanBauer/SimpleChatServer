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
        {
            SERVER_INFO("User document could not be created");
            return false;
        }

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteUser(const std::string& username)
{
    SERVER_INFO("MongoDbHandle::deleteUser");
    try
    {
        // Go through user's server list and remove them from each server
        // Delete any servers the user is the owner of
        // Delete any channels the user's servers' has
        // Delete user




        //mongocxx::collection collection = m_db[k_usersCollection];

        //// Define the filter document
        //auto filter = bsoncxx::builder::stream::document{}
        //    << "username" << username
        //    << bsoncxx::builder::stream::finalize;

        //// Perform the delete_one operation
        //auto deletionResult = collection.delete_one(filter.view());

        //// Check if a document was deleted
        //if (!deletionResult)
        //{
        //    SERVER_INFO("No document matched the filter criteria.");
        //    return false;
        //}

        //SERVER_INFO("Deleted " << deletionResult->deleted_count() << " document(s).");
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
        {
            SERVER_INFO("Username does not exist.");
            return false;
        }

        SERVER_INFO("Username found");

        // Get salt from the returned document
        auto doc = *findResult;
        auto salt = doc["salt"].get_string();

        // Hash given password with salt see if it matches password_hash in retrieved document
        if (doc["password_hash"].get_string().value != hashPassword(password, std::string(salt.value)))
        {
            SERVER_INFO("Incorrect Password");
            return false;
        }

        SERVER_INFO("Correct Password");

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
        {
            SERVER_INFO("No documents matched the query.");
            return false;
        }

        SERVER_INFO("Document updated successfully.");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::logout(const std::string& username)
{
    // Update user document status
    SERVER_INFO("MongoDbHandle::logout");
    try
    {
        // Define the filter to find the document to update
        auto filter = bsoncxx::builder::stream::document{}
            << "username" << username
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
        {
            SERVER_INFO("No documents matched the query.");
            return false;
        }

        SERVER_INFO("Successfully logged out.");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createServer(const std::string& username, const std::string& serverName)
{
    SERVER_INFO("MongoDbHandle::createServer");
    createServerDoc();
    createChannelDoc();
/*
    // Create Server
    // Add Channel to server
    // Add server to owner's server list
    try
    {
        std::string defaultChannelName = "Home";

        // Initialize empty members array
        bsoncxx::builder::basic::array membersArr = bsoncxx::builder::basic::array{};
        membersArr.append(username);

        // Initialize channels array
        bsoncxx::builder::basic::array channelsArr = bsoncxx::builder::basic::array{};
        channelsArr.append(defaultChannelName);

        // Prepare server document
        auto newServerDoc = bsoncxx::builder::stream::document{}
            << "owner_id"   << username
            << "name"       << serverName
            << "members"    << membersArr
            << "channels"   << channelsArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform server document insertion
        auto serverCreationResult = insertOneWithRetry(m_serverCollection, newServerDoc.view());
        if (!serverCreationResult)
        {
            SERVER_INFO("Failed to create server.");
            return false;
        }
        SERVER_INFO("Server successfully created");


        // Add first channel to the server
        if (!addRemoveChannelFromServer(serverName, defaultChannelName, "$push"))
            return false;

        // Add server to user's server list
        if (!addRemoveServerFromMember(username, serverName, "$push"))
            return false;

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
*/

    return true;
}

bool MongoDbHandler::deleteServer(std::string serverName)
{
    SERVER_INFO("MongoDbHandle::deleteServer");
    deleteMessageDocs();
    deleteChannelDocs();
    removeServerFromAllMembers();
    deleteServerDoc();
/*
    try
    {
        bool retVal = true;

        // Define the filter document
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << serverName
            << bsoncxx::builder::stream::finalize;

        // Find the document
        auto result = findOneWithRetry(m_serverCollection, filter.view());
        if (!result)
        {
            SERVER_INFO("Server document not found.");
            retVal = false;
        }

        // Perform the delete_one operation
        auto deleteResult = deleteOneWithRetry(m_serverCollection, filter.view());
        if (!deleteResult)
        {
            SERVER_INFO("No server matched the filter criteria.");
            retVal = false;
        }

        //SERVER_INFO("Found document: {}", bsoncxx::to_json(result->view().data()));
        SERVER_INFO("Deleted server.");

        // Go through the members list and remove the server from each users' server list
        // Go through channels list and delete the channels documents

        return retVal;
        
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
*/

    return true;
}

bool MongoDbHandler::joinServer(std::string serverName, std::string username)
{
    SERVER_INFO("MongoDbHandle::joinServer");
    addRemoveMemberFromServer();
    addRemoveServerFromUser();
    return true;
}

bool MongoDbHandler::leaveServer(std::string serverName, std::string username)
{
    SERVER_INFO("MongoDbHandle::leaveServer");
    addRemoveMemberFromServer();
    addRemoveServerFromUser();
    return true;
}

bool MongoDbHandler::createChannel(std::string serverName, std::string channelName)
{
    SERVER_INFO("MongoDbHandle::createChannel");
    createChannelDoc();
    addRemoveChannelFromServer();
/*
    try
    {
        // Create the home channel for the created server
        // Initialize messages array
        bsoncxx::builder::basic::array messagesArr = bsoncxx::builder::basic::array{};

        // Prepare channel document
        auto newChannelDoc = bsoncxx::builder::stream::document{}
            << "server" << serverName
            << "name" << channelName
            << "messages" << messagesArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!insertOneWithRetry(m_channelCollection, newChannelDoc.view()))
        {
            SERVER_INFO("Failed to create channel.");
            return false;
        }

        SERVER_INFO("Channel successfully created");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
*/
    return true;
}

bool MongoDbHandler::deleteChannel(std::string serverName, std::string channelName)
{
    SERVER_INFO("MongoDbHandle::deleteChannel");
    deleteMessageDocs();
    deleteChannelDoc();
/*
    try
    {
        // Delete channel document
        // Prepare channel deletion document
        auto deleteChannelDoc = bsoncxx::builder::stream::document{}
            << "server" << serverName
            << "name" << channelName
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!deleteOneWithRetry(m_channelCollection, deleteChannelDoc.view()))
        {
            SERVER_INFO("Failed to delete channel.");
            return false;
        }

        SERVER_INFO("Channel successfully deleted");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
*/
    return true;
}

bool MongoDbHandler::sendMessage(const std::string& username, const std::string& server, const std::string& channel, const std::string& message)
{
    SERVER_INFO("MongoDbHandle::sendMessage");
    createMessageDoc();
    addRemoveMessageFromChannel();
    return true;
}

bool MongoDbHandler::deleteMessage(const std::string& id)
{
    SERVER_INFO("MongoDbHandle::deleteMessage");
    deleteMessageDoc();
    addRemoveMessageFromChannel();
    return true;
}

bool MongoDbHandler::editMessage(const std::string& id, const std::string& message)
{
    SERVER_INFO("MongoDbHandle::editMessage");
    try
    {
        // Define document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << id
            << bsoncxx::builder::stream::finalize;
        
        auto update = bsoncxx::builder::stream::document{}
            << "$set"
            << bsoncxx::builder::stream::open_document
            << "content" << message
            << "edited_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!updateOneWithRetry(m_messageCollection, filter.view(), update.view()))
        {
            SERVER_INFO("Message document could not be created");
            return false;
        }

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}



bool MongoDbHandler::createServerDoc(const std::string& serverName, const std::string& ownerId)
{
    SERVER_INFO("MongoDbHandle::createServerDoc");
    try
    {
        std::string defaultChannelName = "Home";

        // Initialize empty members array
        bsoncxx::builder::basic::array membersArr = bsoncxx::builder::basic::array{};
        membersArr.append(ownerId);

        // Initialize channels array
        bsoncxx::builder::basic::array channelsArr = bsoncxx::builder::basic::array{};
        channelsArr.append(defaultChannelName);

        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "name"       << serverName
            << "owner_id"   << ownerId
            << "members"    << membersArr
            << "channels"   << channelsArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!insertOneWithRetry(m_serverCollection, newDoc.view()))
        {
            SERVER_INFO("Failed to create server doc.");
            return false;
        }

        SERVER_INFO("Server doc successfully created");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteServerDoc(const std::string& serverId)
{
    SERVER_INFO("MongoDbHandle::deleteServerDoc");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << serverId
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!deleteOneWithRetry(m_serverCollection, filter.view()))
        {
            SERVER_INFO("Failed to delete server doc.");
            return false;
        }

        SERVER_INFO("Server doc successfully deleted");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createChannelDoc(const std::string& serverId, const std::string& channelName)
{
    SERVER_INFO("MongoDbHandle::createChannelDoc");
    try
    {
        // Initialize empty messages array
        bsoncxx::builder::basic::array messagesArr = bsoncxx::builder::basic::array{};
        messagesArr.append();

        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "server_id" << serverId
            << "name" << channelName
            << "messages" << messagesArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!insertOneWithRetry(m_channelCollection, newDoc.view()))
        {
            SERVER_INFO("Failed to create channel doc.");
            return false;
        }

        SERVER_INFO("Channel doc successfully created");
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
            << "_id" << channelId
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteOneWithRetry(m_channelCollection, filter.view()))
        {
            SERVER_INFO("Failed to delete channel doc.");
            return false;
        }

        SERVER_INFO("Channel doc successfully deleted");
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
            << "server_id" << serverId
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteManyWithRetry(m_channelCollection, filter.view()))
        {
            SERVER_INFO("Failed to delete channel docs.");
            return false;
        }

        SERVER_INFO("Channel docs successfully deleted");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::createMessageDoc(const std::string& channelId, const std::string& authorId, const std::string& content)
{
    SERVER_INFO("MongoDbHandle::createMessageDoc");
    try
    {
        // Prepare document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "channel_id" << channelId
            << "author" << authorId
            << "context" << content
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        if (!insertOneWithRetry(m_channelCollection, newDoc.view()))
        {
            SERVER_INFO("Failed to create message doc.");
            return false;
        }

        SERVER_INFO("Message doc successfully created");
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
            << "_id" << messageId
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteOneWithRetry(m_channelCollection, filter.view()))
        {
            SERVER_INFO("Failed to delete message doc.");
            return false;
        }

        SERVER_INFO("Message doc successfully deleted");
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
            << "channel_id" << channelId
            << bsoncxx::builder::stream::finalize;

        // Perform deletion
        if (!deleteManyWithRetry(m_channelCollection, filter.view()))
        {
            SERVER_INFO("Failed to delete message docs.");
            return false;
        }

        SERVER_INFO("Message docs successfully deleted");
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
        {
            SERVER_INFO("Failed to remove server from members.");
            return false;
        }

        SERVER_INFO("Server successfully removed from members");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveMemberFromServer(const std::string& serverId, const std::string& userId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveMemberFromServer");
    try
    {
        // Prepare filter
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << serverId
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
        {
            SERVER_INFO("No documents matched the filter");
            return false;
        }

        SERVER_INFO("Member successfully {} server", action == "$push" ? "added to" : "removed from");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addRemoveServerFromUser(const std::string& userId, const std::string& serverId, const std::string& action)
{
    SERVER_INFO("MongoDbHandle::addRemoveServerFromUser");
    try
    {
        // Prepare document
        auto filter = bsoncxx::builder::stream::document{}
            << "_id" << userId
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "servers" << serverId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_userCollection, filter.view(), update.view()))
        {
            SERVER_INFO("No documents matched the filter");
            return false;
        }

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
            << "_id" << serverId
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "channels" << channelId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_serverCollection, filter.view(), update.view()))
        {
            SERVER_INFO("No documents matched the filter");
            return false;
        }

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
            << "_id" << channelId
            << bsoncxx::builder::stream::finalize;

        // Prepare update
        auto update = bsoncxx::builder::stream::document{}
            << action
            << bsoncxx::builder::stream::open_document
            << "messages" << messageId
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform update
        if (!updateOneWithRetry(m_channelCollection, filter.view(), update.view()))
        {
            SERVER_INFO("No documents matched the filter");
            return false;
        }

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
    int attempt = 0;
    findOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find_one(filter);
            SERVER_INFO("Document found successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

findManyResult MongoDbHandler::findManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    findManyResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find(filter);
            SERVER_INFO("Documents found successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

insertOneResult MongoDbHandler::insertOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& document,
                                                   int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    insertOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.insert_one(document);
            SERVER_INFO("Document inserted successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

insertManyResult MongoDbHandler::insertManyWithRetry(mongocxx::collection& collection, const std::vector<bsoncxx::document::view>& documents,
                                                     int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    insertManyResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.insert_many(documents);
            SERVER_INFO("Documents inserted successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

updateResult MongoDbHandler::updateOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    updateResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.update_one(filter, update);
            SERVER_INFO("Document updated successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

findOneResult MongoDbHandler::findOneAndUpdateWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                        int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    findOneResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.find_one_and_update(filter, update);
            SERVER_INFO("Document updated successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}


updateResult MongoDbHandler::updateManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    updateResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.update_many(filter, update);
            SERVER_INFO("Document updated successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

deleteResult MongoDbHandler::deleteOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    deleteResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.delete_one(filter);
            SERVER_INFO("Document deleted successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}

deleteResult MongoDbHandler::deleteManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    deleteResult result;
    while (attempt < max_retries)
    {
        try
        {
            result = collection.delete_many(filter);
            SERVER_INFO("Documents deleted successfully.");
            if (result) break;
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
            attempt++;
        }
    }
    return result;
}