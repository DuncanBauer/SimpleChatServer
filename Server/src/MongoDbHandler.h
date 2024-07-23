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
//    "username"         : "username",
//    "password_hash"    : "hashed_password",
//    "salt"             : "salt",
//    "servers"          : ["server_id_1", "server_id_2"],
//    "created_at"       : "timestamp",
//    "last_login"       : "timestamp",
//    "status"           : "string",         // online, offline
//    "displayed_status" : "string"          // online, offline, busy, etc.
//}
// 
//Servers collection
//{
//    "_id"              : "server_id",
//    "name"             : "server_name",
//    "owner_id"         : "user_id",
//    "members"          : ["user_id_1", "user_id_2"],
//    "channels"         : ["channel_id_1", "channel_id_2"],
//    "created_at"       : "timestamp"
//}
// 
//Channels collection
//{
//    "_id"              : "channel_id",
//    "server_id"        : "server_id",
//    "name"             : "channel_name",
//    "messages"         : ["message_id_1", "message_id_2"],
//    "created_at"       : "timestamp"
//}
// 
//Messages collection
//{
//    "_id"              : "message_id",
//    "author"           : "user_id",
//    "channel_id"       : "channel_id",
//    "content"          : "message_content",
//    "created_at"       : "timestamp"
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
    
    bool createUser(const std::string& username, const std::string& password);
    bool deleteUser(const std::string& username);

    bool login(const std::string& username, const std::string& password);
    bool logout(const std::string& username);

    bool createServer(const std::string& ownerId, const std::string& serverName);
    bool deleteServer(const std::string& serverId);

    bool joinServer(const std::string& serverId, const std::string& userId);
    bool leaveServer(const std::string& serverId, const std::string& userId);

    bool createChannel(const std::string& serverId, const std::string& channelName);
    bool deleteChannel(const std::string& channelId);

    bool sendMessage(const std::string& authorId, const std::string& channelId, const std::string& content);
    bool deleteMessage(const std::string& channelId, const std::string& messageId);
    bool editMessage(const std::string& messageId, const std::string& content);

    bool getServerChannels();
    bool getServerMembers();
    bool getChannelMessages();

private:
    bool createServerDoc(const std::string& ownerId, const std::string& serverName, std::string& serverId);
    bool deleteServerDoc(const std::string& serverId, std::vector<std::string>& channelIds, std::vector<std::string>& memberIds);
    
    bool createChannelDoc(const std::string& serverId, const std::string& channelName, std::string& channelId);
    bool deleteChannelDoc(const std::string& channelId);
    bool deleteChannelDocs(const std::string& serverId);
    
    bool createMessageDoc(const std::string& channelId, const std::string& authorId, const std::string& content, std::string& messageId);
    bool deleteMessageDoc(const std::string& messageId);
    bool deleteChannelMessageDocs(const std::string& channelId);

    bool removeServerFromAllMembers(const std::vector<std::string>& members, const std::string& serverId);
    bool removeUserFromAllServers();

    bool addRemoveMemberFromServer(const std::string& serverId, const std::string& userId, const std::string& action); // Action is $push or $pull
    bool addRemoveServerFromUser(const std::string& userId, const std::string& serverId, const std::string& action); // Action is $push or $pull
    bool addRemoveChannelFromServer(const std::string& serverId, const std::string& channelId, const std::string& action); // Action is $push or $pull
    bool addRemoveMessageFromChannel(const std::string& channelId, const std::string& messageId, const std::string& action); // Action is $push or $pull


    //bool deleteChannels(std::string serverName);
    









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
    findOneResult findOneAndUpdateWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update,
                                            int max_retries = 3, int retry_interval_ms = 1000);
    deleteResult deleteOneWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                    int max_retries = 3, int retry_interval_ms = 1000);
    deleteResult deleteManyWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                     int max_retries = 3, int retry_interval_ms = 1000);
    findOneResult findOneAndDeleteWithRetry(mongocxx::collection& collection, const bsoncxx::document::view& filter,
                                            int max_retries = 3, int retry_interval_ms = 1000);

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