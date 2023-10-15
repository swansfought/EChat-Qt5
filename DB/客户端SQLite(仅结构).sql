/*
 Navicat Premium Data Transfer

 Source Server         : ChatMsg
 Source Server Type    : SQLite
 Source Server Version : 3030001
 Source Schema         : main

 Target Server Type    : SQLite
 Target Server Version : 3030001
 File Encoding         : 65001

 Date: 04/06/2023 17:17:59
*/

PRAGMA foreign_keys = false;

-- ----------------------------
-- Table structure for ec_messages
-- ----------------------------
DROP TABLE IF EXISTS "ec_messages";
CREATE TABLE "ec_messages" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "sender" int NOT NULL,
  "senderPicture" int DEFAULT NULL,
  "frd_id" int DEFAULT NULL,
  "grp_id" int DEFAULT NULL,
  "type" tinyint(4) NOT NULL,
  "message" longtext DEFAULT NULL,
  "msg_type" varchar(50) NOT NULL,
  "send_time" timestamp NOT NULL DEFAULT CURRENT_timestamp,
  "file_name" varchar(255) DEFAULT NULL,
  "suffix" varchar(20) DEFAULT NULL,
  "file_path" text DEFAULT NULL
);

-- ----------------------------
-- Table structure for ec_users
-- ----------------------------
DROP TABLE IF EXISTS "ec_users";
CREATE TABLE "ec_users" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "usr_id" int NOT NULL,
  "usr_type" tinyint(4) NOT NULL,
  "usr_nickname" int DEFAULT NULL,
  "usr_picture" int DEFAULT NULL
);

-- ----------------------------
-- Table structure for sqlite_sequence
-- ----------------------------
DROP TABLE IF EXISTS "sqlite_sequence";
CREATE TABLE "sqlite_sequence" (
  "name" ,
  "seq" 
);

-- ----------------------------
-- Auto increment value for ec_messages
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 554 WHERE name = 'ec_messages';

-- ----------------------------
-- Auto increment value for ec_users
-- ----------------------------
UPDATE "sqlite_sequence" SET seq = 43 WHERE name = 'ec_users';

PRAGMA foreign_keys = true;
