#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include "logging/Logger.h"
#include "MongoDbHandler.h"
#include "Util.h"

findOneResult MongoDbHandler::findOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                            int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.find_one(filter);
            std::cout << "Document inserted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

findManyResult MongoDbHandler::findManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                             int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.find(filter);
            std::cout << "Documents inserted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

insertOneResult MongoDbHandler::insertOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& document,
                                                int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries) 
    {
        try
        {
            auto result = collection.insert_one(document);
            std::cout << "Document inserted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

insertManyResult MongoDbHandler::insertManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& documents,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.insert_many(documents);
            std::cout << "Documents inserted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

updateResult MongoDbHandler::updateOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.update_one(filter, update);
            std::cout << "Document updated successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

updateResult MongoDbHandler::updateManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.update_many(filter, update);
            std::cout << "Document updated successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

deleteResult MongoDbHandler::deleteOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries) 
    {
        try 
        {
            auto result = collection.delete_one(filter);
            std::cout << "Document deleted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

deleteResult MongoDbHandler::deleteManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                                 int max_retries, int retry_interval_ms)
{
    int attempt = 0;
    while (attempt < max_retries)
    {
        try
        {
            auto result = collection.delete_many(filter);
            std::cout << "Documents deleted successfully.\n";
            return result;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Insert attempt " << (attempt + 1) << " failed: " << e.what() << '\n';
            if (attempt + 1 >= max_retries)
            {
                std::cerr << "Maximum retries reached. Giving up.\n";
                throw;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
            attempt++;
        }
    }
}

bool MongoDbHandler::registerUser(const std::string& username, const std::string& password)
{
    // Create user document
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
            std::cout << "User document could not be created\n";
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
        //    std::cout << "No document matched the filter criteria." << '\n';
        //    return false;
        //}

        //std::cout << "Deleted " << deletionResult->deleted_count() << " document(s)." << '\n';
        //return true;
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::login(const std::string& username, const std::string& password)
{
    try
    {
        // Define the query document to find the user
        auto query = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Perform the find_one operation to check if the username exists
        auto result = m_userCollection.find_one(query.view());
        if (result)
        {
            // Get salt from the returned document
            auto doc = *result;
            auto salt = doc["salt"].get_string();

            // Hash given password with salt see if it matches password_hash in retrieved document
            if (doc["password_hash"].get_string().value == hashPassword(password, std::string(salt.value)))
            {
                std::cout << "Correct Password\n";

                // Define the filter to find the document to update
                auto filter = bsoncxx::builder::stream::document{}
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

                // Perform the update operation
                
                auto updateResult = updateOneWithRetry(m_userCollection, filter.view(), update.view());
                if (updateResult)
                {
                    if (updateResult->modified_count() > 0)
                        std::cout << "Document updated successfully.\n";
                    else
                        std::cout << "No documents matched the query.\n";
                }
                return true;
            }
            else
            {
                std::cout << "Incorrect Password\n";
                return false;
            }
        }
        else
        {
            std::cout << "Username does not exist.\n";
            return false;
        }
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::logout(const std::string& username)
{
    try
    {
        mongocxx::collection collection = m_db[k_usersCollection];

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
        auto updateResult = collection.update_one(filter.view(), update.view());
        if (updateResult)
        {
            if (updateResult->modified_count() > 0)
                std::cout << "Successfully logged out.\n";
            else
                std::cout << "No documents matched the query.\n";
        }
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
    try
    {
        // Create Server
        // Add Channel to server
        // Add server to owner's server list
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

        // Check if a server was successfully created
        if (!serverCreationResult)
        {
            std::cout << "Failed to create server.\n";
            return false;
        }
        std::cout << "Server successfully created\n";

        // Add first channel to the server
        addChannelToServer(serverName, defaultChannelName);




        // Add server to user's server list
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
            std::cout << "No documents matched the filter" << std::endl;
        }

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
    try
    {
        mongocxx::collection collection = m_db[k_serversCollection];

        // Define the filter document
        auto filter = bsoncxx::builder::stream::document{}
            << "name" << serverName
            << bsoncxx::builder::stream::finalize;

        // Find the document
        auto document = collection.find_one(filter.view());

        if (document) 
        {
            std::cout << "Found document: " << bsoncxx::to_json(document->view()) << std::endl;

            // Perform the delete_one operation
            auto deleteResult = collection.delete_one(filter.view());

            // Check if a server was deleted
            if (deleteResult)
            {
                std::cout << "Deleted server." << '\n';

                // Go through the members list and remove the server from each users' server list
                // Go through channels list and delete the channels documents

                return true;
            }
            else
            {
                std::cout << "No server matched the filter criteria." << '\n';
                return false;
            }
        }
        else 
        {
            std::cout << "Server document not found." << std::endl;
        }
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addChannelToServer(std::string serverName, std::string channelName)
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
        std::cout << "Failed to create channel.\n";
        return false;
    }
    std::cout << "Channel successfully created\n";
}

bool MongoDbHandler::removeChannelFromServer(std::string serverName, std::string channelName)
{
    return true;
}

bool MongoDbHandler::joinServer(std::string serverName, std::string username)
{
    return true;
}

bool MongoDbHandler::leaveServer(std::string serverName, std::string username)
{
    return true;
}

bool MongoDbHandler::sendMessage()
{
    return true;
}

bool MongoDbHandler::editMessage()
{
    return true;
}

bool MongoDbHandler::deleteMessage()
{
    return true;
}