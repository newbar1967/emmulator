#pragma once

class QString;

namespace cfg {

class Storage {
public:
    virtual ~Storage() = default;
    virtual bool Load(const QString& file_name) = 0;
    virtual bool Save(const QString& file_name) = 0;
    virtual bool LoadViaFileDialog() = 0;
    virtual void SaveViaFileDialog() = 0;

protected:
    Storage() = default;
};

} // namespace cfg
