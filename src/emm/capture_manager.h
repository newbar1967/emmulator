#pragma once

#include <unordered_map>
#include <QString>
#include "capture_answer_controller.h"

namespace emm {

class CaptureManager final {
public:
    CaptureManager() = default;

    CaptureManager(CaptureManager& other) = delete;
    CaptureManager& operator=(CaptureManager& other) = delete;
    CaptureManager(CaptureManager&& other) = delete;
    CaptureManager& operator=(CaptureManager&& other) = delete;

    void Insert(const QString& key, emm::CaptureAnswerController* obj);
    emm::CaptureAnswerController* Find(const QString& key) const;
    bool RemoveKey(const QString& key);
    bool RemoveObj(emm::CaptureAnswerController* obj);
    void Clear();

private:
    std::unordered_map<QString, emm::CaptureAnswerController*> key_to_object_;
    std::unordered_map<emm::CaptureAnswerController*, QString> object_to_key_;
};

} // namespace emm

