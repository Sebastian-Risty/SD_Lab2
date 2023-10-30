#include "sms.h"

void SendSMS()
{
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer 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");  // add valid sendgrid API key + change email in payload to the one linked in sendgrid account
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Getting current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::tm timeinfo;
        localtime_s(&timeinfo, &now_time);

        std::ostringstream timeStream;
        timeStream << "Critical Safety Event at "
            << std::setw(2) << std::setfill('0') << timeinfo.tm_hour << ":"
            << std::setw(2) << std::setfill('0') << timeinfo.tm_min << " on "
            << std::setw(2) << std::setfill('0') << (timeinfo.tm_mon + 1) << "/"
            << std::setw(2) << std::setfill('0') << timeinfo.tm_mday << "/"
            << (timeinfo.tm_year + 1900);

        std::string formatted_time = timeStream.str();

        ostringstream payloadStream;
        payloadStream << R"({
            "personalizations": [{
                "to": [{
                    "email": ")" << g_globals.phoneNumber << R"(@)" << g_globals.carrier << R"("
                }]
            }],
            "from": {
                "email": "sristy@uiowa.edu"
            },
            "subject": "SMS Notification",
            "content": [{
                "type": "text/plain",
                "value": ")" << formatted_time << R"("
            }]
        })";

        string payload = payloadStream.str();

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.sendgrid.com/v3/mail/send");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        else
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            cout << "Response code: " << response_code << endl;
            cout << "Response: " << readBuffer << endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}