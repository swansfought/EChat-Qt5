/*
 Navicat Premium Data Transfer

 Source Server         : Tencent
 Source Server Type    : MySQL
 Source Server Version : 50651
 Source Host           : localhost:3306
 Source Schema         : echat

 Target Server Type    : MySQL
 Target Server Version : 50651
 File Encoding         : 65001

 Date: 04/06/2023 17:26:12
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for ec_apply
-- ----------------------------
DROP TABLE IF EXISTS `ec_apply`;
CREATE TABLE `ec_apply`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `applicant` int(11) NOT NULL COMMENT '申请者id，外键',
  `reviewer` int(11) NULL DEFAULT NULL COMMENT '同意者id，外键',
  `type` tinyint(4) NOT NULL COMMENT '类型，0为好友申请，1为群聊申请',
  `grp_id` int(11) NULL DEFAULT NULL COMMENT '群id，外键',
  `apply_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '申请时间',
  `ps` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '附言',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `applicant`(`applicant`) USING BTREE,
  INDEX `reviewer`(`reviewer`) USING BTREE,
  CONSTRAINT `ec_apply_ibfk_1` FOREIGN KEY (`applicant`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_apply_ibfk_2` FOREIGN KEY (`reviewer`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 41 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_chat_objects
-- ----------------------------
DROP TABLE IF EXISTS `ec_chat_objects`;
CREATE TABLE `ec_chat_objects`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '用户id，外键',
  `frd_id` int(11) NULL DEFAULT NULL COMMENT '好友id',
  `grp_id` int(11) NULL DEFAULT NULL COMMENT '群聊id',
  `type` tinyint(4) NOT NULL COMMENT '0-好友  1-群聊',
  `recent_chat_time` datetime(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '最近聊天时间',
  `chat_top` tinyint(4) NOT NULL COMMENT '是否置顶',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `usr_id`(`usr_id`) USING BTREE,
  INDEX `frd_id`(`frd_id`) USING BTREE,
  INDEX `grp_id`(`grp_id`) USING BTREE,
  CONSTRAINT `ec_chat_objects_ibfk_1` FOREIGN KEY (`usr_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_chat_objects_ibfk_2` FOREIGN KEY (`frd_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_chat_objects_ibfk_3` FOREIGN KEY (`grp_id`) REFERENCES `ec_groups` (`grp_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 402 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_friends
-- ----------------------------
DROP TABLE IF EXISTS `ec_friends`;
CREATE TABLE `ec_friends`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '用户id，外键',
  `frd_id` int(11) NULL DEFAULT NULL COMMENT '好友id，外键',
  `build_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '建立好友关系时间',
  `classify` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '好友分类',
  `remark` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '好友备注',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `usr_id`(`usr_id`) USING BTREE,
  INDEX `frd_id`(`frd_id`) USING BTREE,
  CONSTRAINT `ec_friends_ibfk_1` FOREIGN KEY (`usr_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_friends_ibfk_2` FOREIGN KEY (`frd_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 71 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_groups
-- ----------------------------
DROP TABLE IF EXISTS `ec_groups`;
CREATE TABLE `ec_groups`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `grp_id` int(11) NOT NULL COMMENT '群id，唯一',
  `grp_leader` int(11) NOT NULL COMMENT '群主id，外键',
  `grp_nickname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '群昵称',
  `grp_build_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '建群时间',
  `grp_picture` int(11) NULL DEFAULT NULL COMMENT '群头像',
  `grp_intro` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '群简介',
  `grp_members` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL COMMENT '群成员json数组形式',
  `grp_admin_num` int(11) NOT NULL COMMENT '群管理员个数',
  `grp_member_num` int(11) NOT NULL COMMENT '群成员个数',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `grp_id`(`grp_id`) USING BTREE,
  INDEX `grp_leader`(`grp_leader`) USING BTREE,
  CONSTRAINT `ec_groups_ibfk_1` FOREIGN KEY (`grp_leader`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 46 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_new_ids
-- ----------------------------
DROP TABLE IF EXISTS `ec_new_ids`;
CREATE TABLE `ec_new_ids`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `new_id` int(11) NOT NULL COMMENT '用户新id，唯一',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `new_id`(`new_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 424241 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_offline_messages
-- ----------------------------
DROP TABLE IF EXISTS `ec_offline_messages`;
CREATE TABLE `ec_offline_messages`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `sender` int(11) NOT NULL COMMENT '发送者id，外键',
  `frd_id` int(11) NULL DEFAULT NULL COMMENT '接收者id，外键',
  `grp_id` int(11) NULL DEFAULT NULL COMMENT '接收者id，外键',
  `type` tinyint(4) NOT NULL COMMENT '0-好友  1-群聊',
  `message` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL COMMENT '消息',
  `msg_type` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '消息类型',
  `send_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '发送时间',
  `file_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '文件名',
  `suffix` varchar(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '文件后缀',
  `file_path` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '文件路径',
  `senderPicture` int(11) NULL DEFAULT NULL COMMENT '发送者头像',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `sender`(`sender`) USING BTREE,
  INDEX `frd_id`(`frd_id`) USING BTREE,
  INDEX `grp_id`(`grp_id`) USING BTREE,
  CONSTRAINT `ec_offline_messages_ibfk_1` FOREIGN KEY (`sender`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_offline_messages_ibfk_2` FOREIGN KEY (`frd_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_offline_messages_ibfk_3` FOREIGN KEY (`grp_id`) REFERENCES `ec_groups` (`grp_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 120 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_online_users
-- ----------------------------
DROP TABLE IF EXISTS `ec_online_users`;
CREATE TABLE `ec_online_users`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '在线id，外键，唯一',
  `login_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '登录时间',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `usr_id`(`usr_id`) USING BTREE,
  CONSTRAINT `ec_online_users_ibfk_1` FOREIGN KEY (`usr_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_recommends
-- ----------------------------
DROP TABLE IF EXISTS `ec_recommends`;
CREATE TABLE `ec_recommends`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '用户新id，唯一',
  `usr_say` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '建议',
  `usr_say_date` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '提交建议时间',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 9 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_user_groups
-- ----------------------------
DROP TABLE IF EXISTS `ec_user_groups`;
CREATE TABLE `ec_user_groups`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '用户id，外键',
  `grp_id` int(11) NOT NULL COMMENT '群id，外键',
  `identify` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '用户身份',
  `usr_groupNickname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '用户群昵称',
  `join_time` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '进群时间',
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `usr_id`(`usr_id`) USING BTREE,
  INDEX `grp_id`(`grp_id`) USING BTREE,
  CONSTRAINT `ec_user_groups_ibfk_1` FOREIGN KEY (`usr_id`) REFERENCES `ec_users` (`usr_id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `ec_user_groups_ibfk_2` FOREIGN KEY (`grp_id`) REFERENCES `ec_groups` (`grp_id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 102 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Table structure for ec_users
-- ----------------------------
DROP TABLE IF EXISTS `ec_users`;
CREATE TABLE `ec_users`  (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增id',
  `usr_id` int(11) NOT NULL COMMENT '用户id，唯一',
  `usr_password` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '用户密码',
  `usr_nickname` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '用户昵称',
  `usr_register_date` timestamp(0) NOT NULL DEFAULT CURRENT_TIMESTAMP(0) COMMENT '用户注册时间',
  `usr_sex` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '用户性别',
  `usr_picture` int(11) NULL DEFAULT NULL COMMENT '用户头像',
  `usr_phone` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '用户手机号码',
  `usr_region` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '用户所在地',
  `usr_email` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL DEFAULT NULL COMMENT '用户邮箱',
  `usr_self_say` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NULL COMMENT '用户个性签名',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `usr_id`(`usr_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 51 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_general_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;
