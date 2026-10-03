// curl
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/md5.h>

#include <json.hpp>

bool bValid = false;

using json = nlohmann::json;

std::string g_Token, g_Auth, g_notify;

std::string getClipboardText() {
    if (!g_App)
        return "";
    auto activity = g_App->activity;
    if (!activity)
        return "";
    auto vm = activity->vm;
    if (!vm)
        return "";
    auto object = activity->clazz;
    if (!object)
        return "";
    std::string result;
    JNIEnv *env;
    vm->AttachCurrentThread(&env, 0); {
        auto ContextClass = env->FindClass("android/content/Context");
        auto getSystemServiceMethod = env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        auto str = env->NewStringUTF("clipboard");
        auto clipboardManager = env->CallObjectMethod(object, getSystemServiceMethod, str);
        env->DeleteLocalRef(str);
        auto ClipboardManagerClass = env->FindClass("android/content/ClipboardManager");
        auto getText = env->GetMethodID(ClipboardManagerClass, "getText", "()Ljava/lang/CharSequence;");
        auto CharSequenceClass = env->FindClass("java/lang/CharSequence");
        auto toStringMethod = env->GetMethodID(CharSequenceClass, "toString", "()Ljava/lang/String;");
        auto text = env->CallObjectMethod(clipboardManager, getText);
        if (text) {
            str = (jstring) env->CallObjectMethod(text, toStringMethod);
            result = env->GetStringUTFChars(str, 0);
            env->DeleteLocalRef(str);
            env->DeleteLocalRef(text);
        }
        env->DeleteLocalRef(CharSequenceClass);
        env->DeleteLocalRef(ClipboardManagerClass);
        env->DeleteLocalRef(clipboardManager);
        env->DeleteLocalRef(ContextClass);
    }
    vm->DetachCurrentThread();
    return result;
}

const char *GetAndroidID(JNIEnv *env, jobject context) {
    jclass contextClass = env->FindClass(("android/content/Context"));
    jmethodID getContentResolverMethod = env->GetMethodID(contextClass, ("getContentResolver"), ("()Landroid/content/ContentResolver;"));
    jclass settingSecureClass = env->FindClass(("android/provider/Settings$Secure"));
    jmethodID getStringMethod = env->GetStaticMethodID(settingSecureClass, ("getString"), ("(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;"));

    auto obj = env->CallObjectMethod(context, getContentResolverMethod);
    auto str = (jstring) env->CallStaticObjectMethod(settingSecureClass, getStringMethod, obj, env->NewStringUTF(("android_id")));
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceModel(JNIEnv *env) {
    jclass buildClass = env->FindClass(("android/os/Build"));
    jfieldID modelId = env->GetStaticFieldID(buildClass, ("MODEL"), ("Ljava/lang/String;"));

    auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceBrand(JNIEnv *env) {
    jclass buildClass = env->FindClass(("android/os/Build"));
    jfieldID modelId = env->GetStaticFieldID(buildClass, ("BRAND"), ("Ljava/lang/String;"));

    auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
    return env->GetStringUTFChars(str, 0);
}

const char *GetPackageName(JNIEnv *env, jobject context) {
    jclass contextClass = env->FindClass(("android/content/Context"));
    jmethodID getPackageNameId = env->GetMethodID(contextClass, ("getPackageName"), ("()Ljava/lang/String;"));

    auto str = (jstring) env->CallObjectMethod(context, getPackageNameId);
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceUniqueIdentifier(JNIEnv *env, const char *uuid) {
    jclass uuidClass = env->FindClass(("java/util/UUID"));
    auto len = strlen(uuid);
    jbyteArray myJByteArray = env->NewByteArray(len);
    env->SetByteArrayRegion(myJByteArray, 0, len, (jbyte *) uuid);
    jmethodID nameUUIDFromBytesMethod = env->GetStaticMethodID(uuidClass, ("nameUUIDFromBytes"), ("([B)Ljava/util/UUID;"));
    jmethodID toStringMethod = env->GetMethodID(uuidClass, ("toString"), ("()Ljava/lang/String;"));

    auto obj = env->CallStaticObjectMethod(uuidClass, nameUUIDFromBytesMethod, myJByteArray);
    auto str = (jstring) env->CallObjectMethod(obj, toStringMethod);
    return env->GetStringUTFChars(str, 0);
}

std::string RSA_Encrypt(const std::string &clear_text, const std::string &pub_key) {
    std::string result;
    BIO *key = BIO_new_mem_buf((unsigned char *) pub_key.c_str(), -1);
    RSA *rsa = RSA_new();
    rsa = PEM_read_bio_RSA_PUBKEY(key, &rsa, NULL, NULL);
    if (!rsa) {
        return "";
    }
    int key_len = RSA_size(rsa);
    int block_len = key_len - 11;
    char *sub_text = new char[key_len + 1];
    memset(sub_text, 0, key_len + 1);
    int ret = 0;
    int pos = 0;
    std::string sub_str;
    while (pos < clear_text.length()) {
        sub_str = clear_text.substr(pos, block_len);
        memset(sub_text, 0, key_len + 1);
        ret = RSA_public_encrypt(sub_str.length(), (const unsigned char *) sub_str.c_str(), (unsigned char *) sub_text, rsa, RSA_PKCS1_PADDING);
        if (ret >= 0) {
            result.append(std::string(sub_text, ret));
        }
        pos += block_len;
    }
    BIO_free_all(key);
    RSA_free(rsa);
    delete[] sub_text;
    return result;
}

std::string RSA_Decrypt(const std::string &cipher_text, const std::string &pub_key) {
    std::string result;
    BIO *key = BIO_new_mem_buf((unsigned char *) pub_key.c_str(), -1);
    RSA *rsa = RSA_new();
    rsa = PEM_read_bio_RSA_PUBKEY(key, &rsa, NULL, NULL);
    if (!rsa) {
        return "";
    }
    int len = RSA_size(rsa);
    char *buf = new char[len + 1];
    memset(buf, 0, len + 1);
    int ret = RSA_public_decrypt(cipher_text.length(), (const unsigned char *) cipher_text.c_str(), (unsigned char *) buf, rsa, RSA_PKCS1_PADDING);
    if (ret >= 0) {
        result.append(std::string(buf, ret));
    }
    free(buf);
    BIO_free_all(key);
    RSA_free(rsa);
    return result;
}

std::string CalcMD5(std::string s) {
    std::string result;
    unsigned char hash[MD5_DIGEST_LENGTH];
    char tmp[4];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, s.c_str(), s.length());
    MD5_Final(hash, &md5);
    for (unsigned char i : hash) {
        sprintf(tmp, "%02x", i);
        result += tmp;
    }
    return result;
}

std::string CalcSHA256(std::string s) {
    std::string result;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char tmp[4];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, s.c_str(), s.length());
    SHA256_Final(hash, &sha256);
    for (unsigned char i : hash) {
        sprintf(tmp, "%02x", i);
        result += tmp;
    }
    return result;
}

std::string RandomString(const int len) {
    static const char alphanumerics[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand((unsigned) time(0) * getpid());
    std::string tmp;
    tmp.reserve(len);
    for (int i = 0; i < len; ++i) {
        tmp += alphanumerics[rand() % (sizeof(alphanumerics) - 1)];
    }
    return tmp;
}

uint8_t PUBLIC_KEY[450] = {0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0xE2, 0xE5, 0xE7, 0xE9, 0xEE, 0x80, 0xF0, 0xF5, 0xE2, 0xEC, 0xE9, 0xE3, 0x80, 0xEB, 0xE5, 0xF9, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0xAA, 0xED, 0xE9, 0xE9, 0xE2, 0xE9, 0xCA, 0xE1, 0xEE, 0xE2, 0xC7, 0xCB, 0xD1, 0xC8, 0xCB, 0xC9, 0xE7, 0x99, 0xD7, 0x90, 0xE2, 0xE1, 0xF1, 0xE5, 0xE6, 0xE1, 0xE1, 0xEF, 0xE3, 0xE1, 0xF1, 0x98, 0xE1, 0xED, 0xE9, 0xE9, 0xE2, 0xE3, 0xC7, 0xEB, 0xE3, 0xE1, 0xF1, 0xE5, 0xE1, 0x91, 0xD4, 0xCF, 0xEA, 0x96, 0xED, 0x95, 0xCB, 0xF4, 0xEA, 0xCD, 0xCA, 0xE2, 0xC9, 0xE6, 0xD0, 0xFA, 0xCE, 0xC8, 0xC3, 0xAA, 0x96, 0x90, 0xCB, 0x8B, 0xD8, 0xE7, 0xF7, 0xD6, 0xC6, 0xE1, 0xC4, 0xD1, 0xD7, 0xD6, 0xE5, 0xEA, 0xE7, 0xE2, 0xED, 0x95, 0x93, 0xDA, 0xEA, 0x94, 0xCF, 0x99, 0xF9, 0xDA, 0x91, 0xF0, 0xC3, 0x90, 0x94, 0xE6, 0xEA, 0x98, 0xF4, 0xF6, 0xFA, 0xE1, 0xE8, 0xC1, 0x95, 0x8F, 0xCB, 0x96, 0xEA, 0xC8, 0xC7, 0xD0, 0x8F, 0xC7, 0xF3, 0xC6, 0xC7, 0xE1, 0xF6, 0x8F, 0xF0, 0xEB, 0xC1, 0xC5, 0xD8, 0xCA, 0xAA, 0x96, 0x90, 0x94, 0x98, 0xD3,
                           0xEA, 0xCE, 0xC9, 0x8B, 0xF9, 0x97, 0xF8, 0xE2, 0xD2, 0xC8, 0xEE, 0xF9, 0x93, 0xE5, 0xD2, 0xC7, 0xF2, 0xEA, 0x96, 0xED, 0xCF, 0xC5, 0xED, 0xCB, 0xDA, 0xF0, 0xE9, 0xEF, 0xD4, 0xEA, 0xE7, 0xCB, 0x90, 0xC7, 0xD8, 0xEF, 0xF7, 0x95, 0x93, 0xC6, 0xCF, 0x8F, 0xD3, 0xC8, 0xEB, 0xC8, 0xCD, 0xF5, 0xC2, 0x96, 0xC7, 0xC7, 0x90, 0xD0, 0xCE, 0xD6, 0xD0, 0x92, 0xE8, 0xAA, 0x98, 0xE3, 0xE9, 0x97, 0xD2, 0x99, 0xFA, 0xEC, 0xEB, 0x96, 0xC7, 0xF3, 0xF9, 0xC4, 0xF6, 0xD0, 0xD5, 0x98, 0xD4, 0xF9, 0xCA, 0xFA, 0xEC, 0xEE, 0xD1, 0xE6, 0x95, 0xF3, 0xF2, 0xCB, 0xCE, 0xE1, 0xE7, 0xED, 0x92, 0x93, 0xC8, 0xC9, 0x8F, 0xF8, 0x8B, 0xCD, 0xF4, 0x92, 0xF8, 0x93, 0xEC, 0xEF, 0x93, 0xF0, 0x91, 0xC8, 0xCB, 0xC1, 0x8F, 0xC7, 0xF5, 0xCD, 0xF5, 0xEB, 0xC5, 0x95, 0xCF, 0xFA, 0xAA, 0x90, 0x8B, 0xEB, 0xC6, 0xF1, 0x90, 0xF4, 0xCA, 0x96, 0x91, 0xF7, 0xCE, 0xE6, 0x93, 0xE7, 0x92, 0xC1, 0x95, 0xF7, 0xF9, 0xE2, 0xD9, 0x96, 0x8F, 0xCA, 0xE7, 0xE5, 0xDA, 0xC8, 0xF1, 0xC5, 0xEC, 0xC7, 0x95, 0x92, 0x91, 0x90,
                           0xF4, 0x99, 0xD9, 0x91, 0xD3, 0xD8, 0xF6, 0xEC, 0xE5, 0xF6, 0xF6, 0xF6, 0x95, 0xD8, 0xC8, 0xCA, 0xF1, 0xE8, 0xE7, 0xE1, 0xD2, 0x95, 0xD7, 0xC6, 0x97, 0xF1, 0xE9, 0xAA, 0xCD, 0x96, 0xF5, 0xC7, 0xCE, 0xF7, 0xEC, 0xD9, 0xCD, 0x8B, 0xDA, 0xC6, 0x99, 0xE9, 0xCA, 0xC6, 0xC9, 0xCE, 0xE2, 0xC7, 0xF2, 0x99, 0xF0, 0xF4, 0x99, 0xC3, 0xE8, 0xFA, 0xF4, 0x93, 0xD0, 0xC4, 0x8B, 0x97, 0xC9, 0xE1, 0x91, 0xE4, 0xF7, 0x93, 0x92, 0xE3, 0x98, 0xC4, 0xD7, 0xF5, 0xEA, 0xD6, 0xE2, 0xF4, 0xF0, 0xCE, 0xD8, 0xDA, 0xEE, 0xC9, 0xEA, 0x96, 0xCF, 0x96, 0xE2, 0x90, 0xF3, 0xCC, 0xAA, 0xD5, 0xD7, 0xE9, 0xE4, 0xE1, 0xF1, 0xE1, 0xE2, 0xAA, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D, 0xE5, 0xEE, 0xE4, 0x80, 0xF0, 0xF5, 0xE2, 0xEC, 0xE9, 0xE3, 0x80, 0xEB, 0xE5, 0xF9, 0x8D, 0x8D, 0x8D, 0x8D, 0x8D};

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;
    mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

std::string Login(const char *user_key) {
    if (!g_App)
        return "Internal Error";
        
    auto activity = g_App->activity;
    if (!activity)
        return "Internal Error";
        
    auto vm = activity->vm;
    if (!vm)
        return "Internal Error";
        
    auto object = activity->clazz;
    if (!object)
        return "Internal Error";
        
    for (auto &pk : PUBLIC_KEY) {
        pk ^= 0xA0;
    }
    
    JNIEnv *env;
    vm->AttachCurrentThread(&env, 0);
    std::string hwid = user_key;
    hwid += GetAndroidID(env, object);
    hwid += GetDeviceModel(env);
    hwid += GetDeviceBrand(env);
    std::string UUID = GetDeviceUniqueIdentifier(env, hwid.c_str());
    vm->DetachCurrentThread();
    
    std::string token = RandomString(32);
    token += std::to_string(time(0));
    std::string token_hash = CalcMD5(token);

    json json_data;
    json_data[("ukey")] = user_key;
    json_data[("hwid")] = UUID;
    json_data[("token")] = token_hash;
    json_data[("game_id")] = ("pubgm");

    struct MemoryStruct chunk{};
    chunk.memory = (char *) malloc(1);
    chunk.size = 0;
    
    std::string errMsg;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, _enc("POST"));
        curl_easy_setopt(curl, CURLOPT_URL, _enc("https://keymagic.xyz/bgmi/public/connect"));

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, _enc("https"));
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, _enc("Content-Type: application/x-www-form-urlencoded"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        char data[4096];
        sprintf(data, _enc("game=PUBG&user_key=%s&serial=%s"), user_key, UUID.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            try {
                json result = json::parse(chunk.memory);
                if (result[("status")] == true) {
                    std::string token = result[("data")][("token")].get<std::string>();
                    time_t rng = result[("data")][("rng")].get<time_t>();
                    g_notify = result["data"]["ModName"].get<std::string>();
                    
                    if (rng + 30 > time(0)) {
                        std::string auth = enc("PUBG");
                        auth += "-";
                        auth += user_key;
                        auth += "-";
                        auth += UUID;
                        auth += "-";
                        auth += enc("Vm8Lk7Uj2JmsjCPVPVjrLa7zgfx3uz9E");
                        std::string outputAuth = CalcMD5(auth);
                        
                        g_Token = token;
                        g_Auth = outputAuth;
                        
                        bValid = g_Token == g_Auth;
                        if (bValid) {
                            // Command
                        }
                    }
                } else {
                    errMsg = result[("reason")].get<std::string>();
                }
            } catch (json::exception &e) {
                errMsg = "{";
                errMsg += e.what();
                errMsg += "}\n{";
                errMsg += chunk.memory;
                errMsg += "}";
            }
        } else {
            errMsg = curl_easy_strerror(res);
        }
    }
    curl_easy_cleanup(curl);
    return bValid ? "OK" : errMsg;
}
