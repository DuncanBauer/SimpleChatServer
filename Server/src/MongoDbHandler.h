#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>

#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>

using seconds_t = std::chrono::seconds;

const std::string k_mongoDbUri = "mongodb://localhost:27017";
const std::string k_database = "chatServer";

const std::string k_userCountCollection = "userCount";
const std::string k_usersCollection = "users";
const std::string k_serversCollection = "servers";
const std::string k_channelsCollection = "channels";     // Channels in a server
const std::string k_messagesCollection = "messages";     // Messages in a channel

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

    void pingDB();

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

    void registerUser(const std::string& username, const std::string& password, const std::string& display_name);
    void deleteUser(const std::string& username);
    bool login(const std::string& username, const std::string& password);
    void logout(const std::string& username);
    void changeDisplayName(const std::string& username, const std::string& newDisplayName);

    void createServer(const std::string& user, const std::string& serverName);
    void deleteServer(std::string serverName);
    void addChannelToServer();
    void removeChannelFromServer();
    void joinServer();
    void leaveServer();


    void sendMessage();
    void editMessage();
    void deleteMessage();

private:
    //mongocxx::instance m_instance{};
    mongocxx::uri m_uri;
    mongocxx::client m_client;
    mongocxx::database m_db;
};