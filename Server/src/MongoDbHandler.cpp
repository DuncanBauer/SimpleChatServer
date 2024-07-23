#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include "logging/Logger.h"
#include "MongoDbHandler.h"
#include "Util.h"

void MongoDbHandler::pingDB()
{
    // Ping the database.
    const auto ping_cmd = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("ping", 1));
    m_db.run_command(ping_cmd.view());
    std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << '\n';
}

bool MongoDbHandler::registerUser(const std::string& username, const std::string& password)
{
    try
    {
        mongocxx::collection collection = m_db[k_usersCollection];

        // Generate salt and hash password
        std::string salt = generateSalt(16);
        std::string hashedPassword = hashPassword(password, salt);

        // Initialize empty server array
        bsoncxx::builder::basic::array serversArr = bsoncxx::builder::basic::array{};

        // Define document
        auto newDoc = bsoncxx::builder::stream::document{}
            << "username" << username
            << "password_hash" << hashedPassword
            << "salt" << salt
            << "servers" << serversArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << "last_login" << 0
            << "status" << (int)UserStatus::OFFLINE
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        collection.insert_one(newDoc.view());
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
        mongocxx::collection collection = m_db[k_usersCollection];

        // Define the filter document
        auto filter = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Perform the delete_one operation
        auto result = collection.delete_one(filter.view());

        // Check if a document was deleted
        if (result)
        {
            std::cout << "Deleted " << result->deleted_count() << " document(s)." << '\n';
            return true;
        }
        else
        {
            std::cout << "No document matched the filter criteria." << '\n';
            return false;
        }
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
        mongocxx::collection collection = m_db[k_usersCollection];

        // Define the query document to find the user
        auto query = bsoncxx::builder::stream::document{}
            << "username" << username
            << bsoncxx::builder::stream::finalize;

        // Perform the find_one operation to check if the username exists
        auto result = collection.find_one(query.view());
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
                auto updateResult = collection.update_one(filter.view(), update.view());
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

bool MongoDbHandler::joinServer()
{
    return true;
}

bool MongoDbHandler::leaveServer()
{
    return true;
}

bool MongoDbHandler::createServer(const std::string& user, const std::string& serverName)
{
    try
    {
        mongocxx::collection collection = m_db[k_serversCollection];

        // Initialize empty members array
        bsoncxx::builder::basic::array membersArr = bsoncxx::builder::basic::array{};
        membersArr.append(user);

        // Initialize channels array
        bsoncxx::builder::basic::array channelsArr = bsoncxx::builder::basic::array{};

        // Prepare document
        auto builder = bsoncxx::builder::stream::document{};
        bsoncxx::document::value newDoc =
            builder << "owner_id" << user
            << "name" << serverName
            << "members" << membersArr
            << "channels" << channelsArr
            << "created_at" << std::to_string(getSecondsSinceEpoch())
            << bsoncxx::builder::stream::finalize;

        // Perform insertion
        auto result = collection.insert_one(newDoc.view());

        // Add server to user's server list

        // Check if a server was created
        if (result)
        {
            std::cout << "Server successfully created\n";
            return true;
        }
        else
        {
            std::cout << "Failed to create server.\n";
            return false;
        }
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

        // Go through the members list and remove the server from each users' server list

        // Perform the delete_one operation
        auto result = collection.delete_one(filter.view());

        // Check if a server was deleted
        if (result)
        {
            std::cout << "Deleted server." << '\n';
            return true;
        }
        else
        {
            std::cout << "No server matched the filter criteria." << '\n';
            return false;
        }
    }
    catch (std::exception& e)
    {
        SERVER_ERROR("{}", e.what());
        return false;
    }
}

bool MongoDbHandler::addChannelToServer()
{
    return true;
}

bool MongoDbHandler::removeChannelFromServer()
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