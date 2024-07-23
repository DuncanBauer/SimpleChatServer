#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include "logging/Logger.h"
#include "MongoDbHandler.h"
#include "Util.h"

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

bool MongoDbHandler::registerUser(const std::string& username, const std::string& password)
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
    // Find user document
    // Check password hash
    // Update last_login and status
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
    // Create Server
    // Add Channel to server
    // Add server to owner's server list
    SERVER_INFO("MongoDbHandle::createServer");
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
        if (!addChannelToServer(serverName, defaultChannelName))
            return false;

        // Add server to user's server list
        if (!addServerToUser(username, serverName))
            return false;

        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::deleteServer(std::string serverName)
{
    SERVER_INFO("MongoDbHandle::deleteServer");
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
}

bool MongoDbHandler::addServerToUser(std::string username, std::string serverName)
{
    // Add server to user's server list
    SERVER_INFO("MongoDbHandle::addServerToUser");
    try
    {
        // Define the filter to find the user document to update server list
        auto filter = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Define the update to update last_login and status
        auto update = bsoncxx::builder::stream::document{}
            << "$push"
            << bsoncxx::builder::stream::open_document
            << "servers" << serverName
            << bsoncxx::builder::stream::close_document
            << bsoncxx::builder::stream::finalize;

        // Perform the update operation
        auto userUpdateResult = updateOneWithRetry(m_userCollection, filter.view(), update.view());
        if (!userUpdateResult)
        {
            SERVER_INFO("No documents matched the filter");
            return false;
        }

        SERVER_INFO("Server successfully added to user");
        return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::removeServerFromUser(std::string username, std::string serverName)
{
    SERVER_INFO("MongoDbHandle::removeServerFromUser");
    return true;
}

bool MongoDbHandler::addChannelToServer(std::string serverName, std::string channelName)
{
    SERVER_INFO("MongoDbHandle::addChannelToServer");
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
        auto channelCreationResult = insertOneWithRetry(m_channelCollection, newChannelDoc.view());
        if (!channelCreationResult)
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
}

bool MongoDbHandler::removeChannelFromServer(std::string serverName, std::string channelName)
{
    SERVER_INFO("MongoDbHandle::removeChannelFromServer");
    return true;
}

bool MongoDbHandler::joinServer(std::string serverName, std::string username)
{
    SERVER_INFO("MongoDbHandle::joinServer");
    return true;
}

bool MongoDbHandler::leaveServer(std::string serverName, std::string username)
{
    SERVER_INFO("MongoDbHandle::leaveServer");
    return true;
}

bool MongoDbHandler::sendMessage()
{
    SERVER_INFO("MongoDbHandle::sendMessage");
    return true;
}

bool MongoDbHandler::editMessage()
{
    SERVER_INFO("MongoDbHandle::editMessage");
    return true;
}

bool MongoDbHandler::deleteMessage()
{
    SERVER_INFO("MongoDbHandle::deleteMessage");
    return true;
}