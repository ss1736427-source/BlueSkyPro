# Journal service tests

Focused coverage for `JournalService::recordUavDecision()`:

- accepts `RETURN`;
- accepts `CONTINUE`;
- rejects missing operator decision;
- rejects unsupported event type;
- rejected events do not enter the store.

The memory store remains a development/test adapter.