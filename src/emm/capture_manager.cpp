#include "capture_manager.h"

namespace emm {

void CaptureManager::Insert(const QString& key, emm::CaptureAnswerController* obj) {
    // Проверяем, существует ли такой указатель.
    auto obj_it = object_to_key_.find(obj);
    if (obj_it != object_to_key_.end()) {
        // Указатель найден — удаляем старую пару.
        const QString& old_key = obj_it->second;
        key_to_object_.erase(old_key); // Удаляем по ключу.
        object_to_key_.erase(obj); // Удаляем по указателю.
    }

    // Вставляем новую пару, только если ключ не пустая строка.
    if (!key.isEmpty()) {
        key_to_object_[key] = obj;
        object_to_key_[obj] = key;
    }
}

CaptureAnswerController* CaptureManager::Find(const QString& key) const {
    auto it = key_to_object_.find(key);
    return (it != key_to_object_.end()) ? it->second : nullptr;
}

bool CaptureManager::RemoveKey(const QString& key) {
    auto key_it = key_to_object_.find(key);
    if (key_it == key_to_object_.end()) {
        return false;
    }

    emm::CaptureAnswerController* obj = key_it->second;

    // Удаляем из обоих контейнеров.
    key_to_object_.erase(key);
    object_to_key_.erase(obj);

    return true;
}

bool CaptureManager::RemoveObj(emm::CaptureAnswerController* obj) {
    auto obj_it = object_to_key_.find(obj);
    if (obj_it == object_to_key_.end()) {
        return false;
    }

    QString key = obj_it->second;

    // Удаляем из обоих контейнеров.
    key_to_object_.erase(key);
    object_to_key_.erase(obj);

    return true;
}

void CaptureManager::Clear() {
    key_to_object_.clear();
    object_to_key_.clear();
}

} // namespace emm


