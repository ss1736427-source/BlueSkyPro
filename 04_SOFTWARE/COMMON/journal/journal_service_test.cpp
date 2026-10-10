#include "journal_service.hpp"
#include "journal_store_memory.hpp"
#include <cassert>

using namespace bluesky::journal;

int main() {
    JournalStoreMemory store;
    JournalService service(store);

    JournalEvent valid{
        "evt-001","UAV_DECISION","2026-09-20T00:00:00Z",
        "BS-260920-A-001","1","UAV-01","battery",
        "reserve and distance","RETURN","RETURN","RETURN","pilot-001","HMI"
    };
    assert(service.recordUavDecision(valid));
    assert(store.recent().size() == 1);

    JournalEvent cont = valid;
    cont.event_id="evt-002"; cont.uav_id="UAV-02";
    cont.operator_decision="CONTINUE";
    cont.system_recommendation="CONTINUE";
    cont.resulting_action="CONTINUE";
    assert(service.recordUavDecision(cont));
    assert(store.recent().size() == 2);

    JournalEvent invalid = valid;
    invalid.event_id="evt-003"; invalid.operator_decision.clear();
    assert(!service.recordUavDecision(invalid));
    assert(store.recent().size() == 2);

    JournalEvent wrong = valid;
    wrong.event_id="evt-004"; wrong.event_type="OTHER_EVENT";
    assert(!service.recordUavDecision(wrong));
    assert(store.recent().size() == 2);
    return 0;
}
