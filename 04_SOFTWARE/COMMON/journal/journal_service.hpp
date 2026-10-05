#pragma once
#include "journal_store.hpp"

namespace bluesky::journal {

class JournalService {
public:
    explicit JournalService(JournalStore& store) : store_(store) {}

    bool recordUavDecision(const JournalEvent& event);

private:
    JournalStore& store_;
};

} // namespace bluesky::journal
