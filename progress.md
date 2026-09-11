
### Define event semantics

Let's first define what each event actually means.
| Event     | Meaning                     | Required information            |
| --------- | --------------------------- | ------------------------------- |
| `Trade`   | A transaction occurred      | price, quantity                 |
| `Quote`   | Current best bid/ask        | bid price/qty, ask price/qty    |
| `Add`     | New order entered book      | order ID, side, price, quantity |
| `Cancel`  | Existing order cancelled    | order ID, quantity              |
| `Execute` | Existing order was executed | order ID, price, quantity       |

### Use a tagged payload

Instead, let's separate the event envelope from the event payload
```yaml
                    MarketEvent
                         │
          ┌──────────────┴──────────────┐
          │                             │
       Envelope                       Payload
          │                             │
   event_id                         TradeEvent
   sequence_number                  QuoteEvent
   timestamp                        AddOrderEvent
   symbol                           CancelOrderEvent
   event_type                       ExecuteEvent
```

### Phase goal: order should have a lifecycle
```yaml
NEW
 │
 ├──> PARTIALLY_FILLED
 │         │
 │         ├──> FILLED
 │         └──> CANCEL_PENDING ──> CANCELLED
 │
 ├──> CANCEL_PENDING ──> CANCELLED
 │
 └──> REJECTED
```
`Market events describe what happened; order state describes the current truth of the order.`