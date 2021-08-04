#include "questionsgenerator.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

QuestionsGenerator::QuestionsGenerator() {}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string QuestionsGenerator::curlCollect(std::string url) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer = "";

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLcode::CURLE_OK)
      return "";
  }
  return readBuffer;
}

std::vector<Question> QuestionsGenerator::collectQuestions() {

  std::vector<Question> questions;

  std::string response = curlCollect(
      "https://opentdb.com/api.php?amount=10&type=multiple&encode=base64");

  if (response == "")
    return {};

  rapidjson::Document d;
  d.Parse(response.c_str());

  // Check if we parsed OK
  if (!d.IsObject())
    return {};

  // Check if we have response_code
  if (!d.HasMember("response_code") || !d["response_code"].IsInt())
    return {};

  // Check if response_code is OK
  if (d["response_code"].GetInt() != 0)
    return {};

  const rapidjson::Value &a = d["results"];
  if (!a.IsArray())
    return {};

  for (rapidjson::SizeType i = 0; i < a.Size(); i++) {
    auto obj = a[i].GetObject();
    auto question = Base64::decode(obj["question"].GetString());
    auto correctAnswer = Base64::decode(obj["correct_answer"].GetString());

    const rapidjson::Value &incorrect_answers = obj["incorrect_answers"];
    if (!incorrect_answers.IsArray())
      return {};

    std::vector<std::string> answers;

    for (rapidjson::SizeType j = 0; j < incorrect_answers.Size(); j++) {
      answers.emplace_back(Base64::decode(incorrect_answers[j].GetString()));
    }

    questions.emplace_back(question, answers[0], answers[1], answers[2],
                           correctAnswer);
  }

  return questions;
}
