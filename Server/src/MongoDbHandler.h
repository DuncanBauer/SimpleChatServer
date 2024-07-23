#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>

#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>

//Users collection
//{
//    "_id": "user_id",
//    "username"        : "username",
//    "password_hash"   : "hashed_password",
//    "salt"            : "salt",
//    "display_name"    : "display_name",
//    "servers"         : ["server_id_1", "server_id_2"] ,
//    "created_at"      : "timestamp",
//    "last_login"      : "timestamp",
//    "status"          : "string",         // online, offline, busy, etc.
//}
// 
//Servers collection
//{
//    "_id": "server_id",
//    "name": "server_name",
//    "owner_id": "user_id",
//    "members": ["user_id_1", "user_id_2"],
//    "channels": ["channel_id_1", "channel_id_2"],
//    "created_at": "timestamp"
//}
// 
//Channels collection
//{
//    "_id": "channel_id",
//    "server_id" : "server_id",
//    "name" : "channel_name",
//    "created_at" : "timestamp"
//}
// 
//Messages collection
//{
//    "_id": "message_id",
//    "channel_id" : "channel_id",
//    "sender_id" : "user_id",
//    "type" : "string", // text, image, video, etc.
//    "content" : "message_content",
//    "timestamp" : "timestamp"
//}

using seconds_t = std::chrono::seconds;

const std::string k_mongoDbUri = "mongodb://localhost:27017";
const std::string k_database = "chatServer";
const std::string k_usersCollection = "users";
const std::string k_serversCollection = "servers";
const std::string k_channelsCollection = "channels"; // Channels in a server
const std::string k_messagesCollection = "messages"; // Messages in a channel

typedef std::optional<bsoncxx::v_noabi::document::value> findOneResult;
typedef std::optional<mongocxx::v_noabi::cursor> findManyResult;
typedef std::optional<mongocxx::v_noabi::result::insert_one> insertOneResult;
typedef std::optional<mongocxx::v_noabi::result::insert_many> insertManyResult;
typedef std::optional<mongocxx::v_noabi::result::update> updateResult;
typedef std::optional<mongocxx::v_noabi::result::delete_result> deleteResult;

enum class UserStatus
{
    OFFLINE,
    ONLINE
};

class MongoDbHandler
{

public:
    MongoDbHandler() : m_uri(k_mongoDbUri), m_client(mongocxx::client(m_uri)), m_db(m_client[k_database]) {}
    ~MongoDbHandler() {}

    findOneResult findOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                   int max_retries = 3, int retry_interval_ms = 1000);
    findManyResult findManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                     int max_retries = 3, int retry_interval_ms = 1000);
    insertOneResult insertOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& document,
                                       int max_retries = 3, int retry_interval_ms = 1000);
    insertManyResult insertManyWithRetry(mongocxx::collection& collection, const std::vector<bsoncxx::document::view>& documents,
                                         int max_retries = 3, int retry_interval_ms = 1000);
    updateResult updateOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                    int max_retries = 3, int retry_interval_ms = 1000);
    updateResult updateManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                     int max_retries = 3, int retry_interval_ms = 1000);
    deleteResult deleteOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                    int max_retries = 3, int retry_interval_ms = 1000);
    deleteResult deleteManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                     int max_retries = 3, int retry_interval_ms = 1000);
    
    //Create user document
    bool registerUser(const std::string& username, const std::string& password);

    // Go through user's server list and remove them from each server
    // Delete any servers the user is the owner of
    // Delete any channels the user's servers' has
    // Delete user
    bool deleteUser(const std::string& username);

    bool login(const std::string& username, const std::string& password);

    bool logout(const std::string& username);

    // Add server to owner's server list
    // Create Server
    // addChannelToServer("Home")
    bool createServer(const std::string& username, const std::string& serverName);

    // Find server
    //   return if fail
    // Delete all channels of server
    //   return if fail
    // Remove server from every members server list
    //   return if fail
    // Delete server
    bool deleteServer(std::string serverName);

    bool addServerToUser(std::string username, std::string serverName);

    bool removeServerFromUser(std::string username, std::string serverName);

    // Find server
    //   return if fail
    // Build create channel document
    //   return if fail
    // Add channel to server doc channel list
    bool addChannelToServer(std::string serverName, std::string channelName);

    // Find server
    //   return if fail
    // Build delete channel document
    //   return if fail
    // Add channel to server doc channel list
    bool removeChannelFromServer(std::string serverName, std::string channelName);

    bool joinServer(std::string serverName, std::string username);

    bool leaveServer(std::string serverName, std::string username);

    bool sendMessage();

    bool editMessage();

    bool deleteMessage();

private:
    //mongocxx::instance m_instance{};
    mongocxx::uri m_uri;
    mongocxx::client m_client;
    mongocxx::database m_db;

    mongocxx::collection m_userCollection    = m_db[k_usersCollection];
    mongocxx::collection m_serverCollection  = m_db[k_serversCollection];
    mongocxx::collection m_channelCollection = m_db[k_channelsCollection];
    mongocxx::collection m_messageCollection = m_db[k_messagesCollection];

};