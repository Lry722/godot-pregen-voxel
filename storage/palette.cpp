#include "palette.h"
#include <cstring>
#include <sstream>

namespace pgvoxel
{

    VoxelData Palette::pick(const size_t index) const
    {
        return index_to_data.at(index);
    }

    size_t Palette::indexOf(const VoxelData data) const
    {
        return std::find_if(index_to_data.begin(), index_to_data.end(), [data](const decltype(index_to_data)::value_type &pair)
                            { return pair.second == data; })
            ->first;
    }

    bool Palette::update(const VoxelData new_data, const VoxelData old_data)
    {
        auto iter = data_to_ref.find(old_data);

        bool removed{false};
        // 这里假设old data是已存在的，否则是调用方出了问题
        if (--(iter->second) == 0)
        {
            // old data的ref归零后要删除对应的Entry
            data_to_ref.erase(iter);
            index_to_data.erase(
                std::find_if(index_to_data.begin(), index_to_data.end(),
                             [old_data](const decltype(index_to_data)::value_type &pair)
                             { return pair.second == old_data; }));
            removed = true;
        }

        bool added{false};
        iter = data_to_ref.find(new_data);
        if (iter != data_to_ref.end())
            // 如果new data已存在，则简单地将引用加一
            ++(iter->second);
        else
        {
            // 如果是new data原先不存在，则要寻找新的index进行插入
            size_t pre = -1;
            auto iter = index_to_data.begin();
            while (iter != index_to_data.end() && iter->first - pre <= 1)
                pre = (iter++)->first;
            size_t new_index = iter->first - 1;

            if (iter != index_to_data.end())
                new_index = iter->first - 1;
            else
                new_index = std::prev(iter)->first + 1;

            index_to_data[new_index] = new_data;
            data_to_ref[new_data] = 1;
            added = true;
        }

        return !removed && added;
    }

    void Palette::serialize(std::ostringstream &oss) const
    {
        for (const auto &elem : index_to_data)
        {
            oss.write(reinterpret_cast<const char *>(&(elem.first)), sizeof(elem.first))
                .write(reinterpret_cast<const char *>(&(elem.second)), sizeof(elem.second))
                .write(reinterpret_cast<const char *>(&(data_to_ref.at(elem.second))), sizeof(data_to_ref.at(elem.second)));
        }
    }

    void Palette::deserialize(std::istringstream &iss, const size_t size)
    {
        size_t index, ref;
        VoxelData data;
        for (int i = 0; i < size / kEntrySize; ++i)
        {
            iss.read(reinterpret_cast<char *>(&(index)), sizeof(index))
                .read(reinterpret_cast<char *>(&(data)), sizeof(data))
                .read(reinterpret_cast<char *>(&(ref)), sizeof(ref));
            index_to_data.insert({index, data});
            data_to_ref.insert({data, ref});
        }
    }

} // namespace lry
