```yaml
                    FRONTEND
               React + TypeScript
                      Vercel
                        |
                        | HTTPS / REST / WebSocket
                        v
                 GO BACKEND
              API + Control Plane
                        |
             +----------+----------+
             |                     |
          gRPC                  gRPC
             |                     |
             v                     v
     C++ MARKET ENGINE       PYTHON ML ENGINE
     ------------------      -----------------
     Market simulation      Feature engineering
     Order lifecycle        Dataset generation
     Event processing       Model training
     Event replay           Model evaluation
     Market state           Model inference
     Execution              Strategy research
             |                     |
             +----------+----------+
                        |
              +---------+---------+
              |                   |
              v                   v
          PostgreSQL            Redis
          ------------          ------------
          Persistent data       Cache
          Experiment metadata    Fast data access
          Model metadata         Transient state
          Historical results    Coordination
```