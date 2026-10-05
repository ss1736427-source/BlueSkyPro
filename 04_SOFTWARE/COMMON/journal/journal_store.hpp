#pragma once
#include "journal_event.hpp"
#include <vector>

namespace bluesky::journal {

class JournalStore {
public:
    virtual ~JournalStore() = default;
    virtual bool append(const JournalEvent& event) = 0;
    virtual std::vector<JournalEvent> recent() const = 0;
};

} // namespace bluesky::journal
