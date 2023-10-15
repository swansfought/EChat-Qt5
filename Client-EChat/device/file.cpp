#include "file.h"

QMutex File::mutex;
File* File::instance=nullptr;

File::File(QObject *parent)
    : QObject{parent}
{

}

File *File::getInstance()
{
   if (instance == nullptr) {
       QMutexLocker locker(&mutex);
       if (instance == nullptr) {
           instance = new File;
       }
   }
   return instance;
}

void File::updateAutoLogin(const bool& state)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("AutoLogin",state);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateRemainPwd(const bool& state)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("RemainPwd",state);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateNewNotifyVoice(const bool& state)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("NewNotifyVoice",state);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateGroupNotifyVoice(const bool& state)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("GroupNotifyVoice",state);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateAutoUpdate(const bool& state)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("AutoUpdate",state);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateFilePath(const QString& filePath)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("FilePath",filePath);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updateEchatId(const int &userId)
{
    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("EchatId",userId); //存储加密之后的
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}

void File::updatePassword(const QString& userPassword)
{
    QString encryPwd = encryptPassword(userPassword); //加密用户密码
    password = encryPwd;

    QSettings newSettings(CONFIG_FILE,QSettings::IniFormat);
    newSettings.setIniCodec("utf-8");//解决乱码
    newSettings.beginGroup("AllConfig");
    newSettings.setValue("EncryptPassword",encryPwd);
    newSettings.endGroup();
    newSettings.sync();//同步
    checkConfigFile(); //变量与配置同步
}


//检查数据文件夹，缺少就创建
void File::checkDataFolder()
{
    auto db = DataBase::getInstance();

    //1.数据文件夹不存在
    QDir dataPath(DATA_PATH);
    if(!dataPath.exists()){
        dataPath.setPath(CURR_PATH);
        dataPath.mkdir(DATA_FOLDER); //创建EChat Data文件夹

        dataPath.setPath(DATA_PATH);
        dataPath.mkdir(USER_FOLDER(selfId)); //创建用户文件夹

        dataPath.setPath(USER_PATH(selfId));
        dataPath.mkdir(AUDIO_FOLDER); //创建Audio文件夹
        dataPath.mkdir(CHATMSG_FOLDER); //创建ChatMsg文件夹
        dataPath.mkdir(FILE_FOLDER);//创建File文件夹
        dataPath.mkdir(SCREENSHOT_FOLDER); //创建ScreenShot文件夹
        dataPath.mkdir(VIDEO_FOLDER); //创建Video文件夹
        dataPath.mkdir(IMAGE_FOLDER);//创建Image文件夹

        dataPath.mkdir(IMAGE_PATH(selfId));
        dataPath.mkdir(IMAGE_HEADPICTURE_FOLDER);//创建HeadPicture文件夹

        db->connectDB(); //连接数据库
        return;
    }

    //2.用户文件夹不存在
    QDir userPath(USER_PATH(selfId));
    if(!userPath.exists()){
        userPath.setPath(DATA_PATH);
        userPath.mkdir(USER_FOLDER(selfId)); //创建用户文件夹

        userPath.setPath(USER_PATH(selfId));
        userPath.mkdir(AUDIO_FOLDER); //创建Audio文件夹
        userPath.mkdir(CHATMSG_FOLDER); //创建ChatMsg文件夹
        userPath.mkdir(FILE_FOLDER);//创建File文件夹
        userPath.mkdir(SCREENSHOT_FOLDER); //创建ScreenShot文件夹
        userPath.mkdir(VIDEO_FOLDER); //创建Video文件夹
        userPath.mkdir(IMAGE_FOLDER);//创建Image文件夹

        userPath.mkdir(IMAGE_PATH(selfId));
        userPath.mkdir(IMAGE_HEADPICTURE_FOLDER);//创建HeadPicture文件夹

        db->connectDB(); //连接数据库
        return;
    }

    //3.音频文件夹不存在
    QDir audioPath(AUDIO_PATH(selfId));
    if(!audioPath.exists()){
        audioPath.setPath(USER_PATH(selfId));
        audioPath.mkdir(AUDIO_FOLDER);
    }

    //4.聊天信息文件夹不存在
    QDir chatMsgPath(CHATMSG_PATH(selfId));
    if(!chatMsgPath.exists()){
        chatMsgPath.setPath(USER_PATH(selfId));
        chatMsgPath.mkdir(CHATMSG_FOLDER);
    }

    //5.文件文件夹不存在
    QDir filePath(FILE_PATH(selfId));
    if(!filePath.exists()){
        filePath.setPath(USER_PATH(selfId));
        filePath.mkdir(FILE_FOLDER);
    }

    //6.图片文件夹不存在
    QDir imagePath(IMAGE_PATH(selfId));
    if(!imagePath.exists()){
        imagePath.setPath(USER_PATH(selfId));
        imagePath.mkdir(IMAGE_FOLDER);

        imagePath.setPath(IMAGE_PATH(selfId));
        imagePath.mkdir(IMAGE_HEADPICTURE_FOLDER);
    }else{
        //6.1.头像文件夹不存在
        QDir headPicturePath(IMAGE_HEADPICTURE_PATH(selfId));
        if(!headPicturePath.exists()){
            headPicturePath.setPath(IMAGE_PATH(selfId));
            headPicturePath.mkdir(IMAGE_HEADPICTURE_FOLDER);
        }
    }

    //7.截图文件夹不存在
    QDir screenShotPath(SCREENSHOT_PATH(selfId));
    if(!screenShotPath.exists()){
        screenShotPath.setPath(USER_PATH(selfId));
        screenShotPath.mkdir(SCREENSHOT_FOLDER);
    }

    //8.视频文件夹不存在
    QDir videoPath(VIDEO_PATH(selfId));
    if(!videoPath.exists()){
        videoPath.setPath(USER_PATH(selfId));
        videoPath.mkdir(VIDEO_FOLDER);
    }

    db->connectDB(); //连接数据库
    return;
}

//检查配置文件，缺少就创建
void File::checkConfigFile()
{
    QFile configFile(CONFIG_FILE);
    //配置文件存在情况
    if(configFile.exists()){
        QSettings getSettings(CONFIG_FILE,QSettings::IniFormat);  //加载配置文件
        getSettings.setIniCodec("utf-8");// 解决乱码
        getSettings.beginGroup("AllConfig");
        AutoLogin = getSettings.value("AutoLogin").toBool();
        RemainPwd = getSettings.value("RemainPwd").toBool();
        NewNotifyVoice = getSettings.value("NewNotifyVoice").toBool();
        GroupNotifyVoice = getSettings.value("GroupNotifyVoice").toBool();
        AutoUpdate = getSettings.value("AutoUpdate").toBool();
        FilePath = getSettings.value("FilePath").toString();

        //自动登录
        echatId = getSettings.value("EchatId").toInt();
        password = getSettings.value("EncryptPassword").toString();
        getSettings.endGroup();
        return;
    }


    //创建新配置文件
    QSettings settings(CONFIG_FILE,QSettings::IniFormat);
    settings.setIniCodec("utf-8");// 解决乱码
    settings.beginGroup("AllConfig");
    settings.setValue("AutoLogin",false);
    settings.setValue("RemainPwd",false);
    settings.setValue("NewNotifyVoice",true);
    settings.setValue("GroupNotifyVoice",true);
    settings.setValue("AutoUpdate",false);
    settings.setValue("FilePath",DATA_PATH);
    settings.setValue("EchatId",0);
    settings.setValue("EncryptPassword","");
    settings.endGroup();

    AutoLogin = false;
    RemainPwd = false;
    NewNotifyVoice = true;
    GroupNotifyVoice = true;
    FilePath = DATA_PATH;
    AutoUpdate = false;

    //自动登录
    echatId = 0;
    password = "";
}

QString File::encryptPassword(const QString &password)
{
    if(password.isEmpty())
        return "";

    static QByteArray key = "H&ide$Pas%swo^rd";

    QByteArray arrayPwd = password.toUtf8();
    int size = arrayPwd.size();
    //加密&解密
    for(int i=0; i<size; ++i){
        arrayPwd[i] = arrayPwd[i] ^ key[i % key.size()];
    }
    return QString::fromUtf8(arrayPwd);
}



