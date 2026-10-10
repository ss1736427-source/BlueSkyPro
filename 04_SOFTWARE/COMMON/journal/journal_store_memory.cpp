#include "journal_store_memory.hpp"

namespace bluesky::journal {

bool JournalStoreMemory::append(const JournalEvent& event) {
    events_.push_back(event);
    return true;
}

std::vector<JournalEvent> JournalStoreMemory::recent() const {
    return events_;
}

} // namespace bluesky::journal
