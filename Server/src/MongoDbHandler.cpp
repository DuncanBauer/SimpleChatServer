#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include "MongoDbHandler.h"
#include "Util.h"

void MongoDbHandler::pingDB()
{
    // Ping the database.
    const auto ping_cmd = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("ping", 1));
    m_db.run_command(ping_cmd.view());
    std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << '\n';
}

void MongoDbHandler::registerUser(const std::string& username, const std::string& password, const std::string& display_name)
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
        << "display_name" << display_name
        << "servers" << serversArr
        << "created_at" << std::to_string(getSecondsSinceEpoch())
        << "last_login" << 0
        << "status" << (int)UserStatus::OFFLINE
        << bsoncxx::builder::stream::finalize;

    // Perform insertion
    collection.insert_one(newDoc.view());
}

void MongoDbHandler::deleteUser(const std::string& username)
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
        std::cout << "Deleted " << result->deleted_count() << " document(s)." << '\n';
    else
        std::cout << "No document matched the filter criteria." << '\n';
}

bool MongoDbHandler::login(const std::string& username, const std::string& password)
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

void MongoDbHandler::logout(const std::string& username)
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
}

void MongoDbHandler::changeDisplayName(const std::string& username, const std::string& newDisplayName)
{
    mongocxx::collection collection = m_db[k_usersCollection];

    // Define the filter document
    auto filter = bsoncxx::builder::stream::document{}
    << "username" << username << bsoncxx::builder::stream::finalize;

    // Define the update
    auto update = bsoncxx::builder::stream::document{}
        << "$set"
        << bsoncxx::builder::stream::open_document
        << "displayName" << newDisplayName
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    // Set options to return the updated document
    mongocxx::options::find_one_and_update options{};
    options.return_document(mongocxx::options::return_document::k_after);

    // Perform the find_one_and_update operation
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        collection.find_one_and_update(filter.view(), update.view(), options);

    // Check if a document was found and updated
    if (result)
        std::cout << bsoncxx::to_json(*result) << '\n';
    else
        std::cout << "No document matched the filter criteria." << '\n';
}

void MongoDbHandler::joinServer()
{}

void MongoDbHandler::leaveServer()
{}

void MongoDbHandler::createServer(const std::string& user, const std::string& serverName)
{
    mongocxx::collection collection = m_db[k_serversCollection];

    // Initialize empty members array
    bsoncxx::builder::basic::array membersArr = bsoncxx::builder::basic::array{};

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

    // Check if a server was created
    if (result)
        std::cout << "Server successfully created\n";
    else
        std::cout << "Failed to create server.\n";
}

void MongoDbHandler::deleteServer(std::string serverName)
{
    mongocxx::collection collection = m_db[k_serversCollection];

    // Define the filter document
    auto filter = bsoncxx::builder::stream::document{}
        << "name" << serverName
        << bsoncxx::builder::stream::finalize;

    // Perform the delete_one operation
    auto result = collection.delete_one(filter.view());

    // Check if a server was deleted
    if (result)
        std::cout << "Deleted server." << '\n';
    else
        std::cout << "No server matched the filter criteria." << '\n';
}

void MongoDbHandler::addChannelToServer()
{}

void MongoDbHandler::removeChannelFromServer()
{}

void MongoDbHandler::sendMessage()
{}

void MongoDbHandler::editMessage()
{}

void MongoDbHandler::deleteMessage()
{}