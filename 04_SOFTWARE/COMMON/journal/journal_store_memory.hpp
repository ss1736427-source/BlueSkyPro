#pragma once
#include "journal_store.hpp"

namespace bluesky::journal {

class JournalStoreMemory final : public JournalStore {
public:
    bool append(const JournalEvent& event) override;
    std::vector<JournalEvent> recent() const override;

private:
    std::vector<JournalEvent> events_;
};

} // namespace bluesky::journal
