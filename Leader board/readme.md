## Leader Board

Great — below is a full, interview-ready plan **and** a C++ implementation sketch you can present in an LLD interview for the million-player leaderboard (write-behind + Redis + Kafka + DB). I’ll give:

1. A crisp interview answer outline you can *speak* (step-by-step).
2. Architecture diagram + sequence flow to draw or describe.
3. Data model and APIs to show on a whiteboard.
4. C++ code skeletons for:

   * `LeaderboardService` (API server side) — updates Redis and publishes events.
   * `EventPublisher` (Kafka producer interface).
   * `DBSyncService` (consumer microservice sketch) — consumes events and does batch DB writes.
   * An in-memory fallback for local testing (to demo in interview).
5. Concurrency, scaling, failure, testing & monitoring talking points.
6. How to demo / what to say about complexity, tradeoffs, and extensions.

Use this as your answer script + code to show you understand both LLD and practical implementation.

---

# 1) What to say at the start of the interview (30–60s elevator)

“First I’ll state requirements (functional + non-functional), then propose a scalable architecture using Redis sorted sets as the real-time store and a write-behind pipeline (Kafka) for durability. I’ll describe how API servers do fast updates to Redis, publish events to Kafka, and a separate DBSync microservice consumes events and persists to a durable DB in batches. I’ll show data structures, API surface, give complexity, and present a C++ implementation sketch for the main pieces (Redis client + Kafka producer + DBSync consumer). I’ll also cover failure modes, consistency, testing and monitoring.”

---

# 2) Requirements (say this clearly)

Functional:

* `AddScore(playerId, delta)` (increment score).
* `GetTopN(n)` (top n players).
* `GetRank(playerId)`.

Non-functional:

* Support millions of players concurrently.
* Low read/write latency (target <100ms).
* Horizontal scalability & high availability.
* Eventual persistence of scores (no catastrophic loss).
* Reasonable cost memory-wise.

Assumptions:

* Some eventual consistency is allowed (ranks may be slightly stale for a few seconds).
* Player identity and metadata are stored separately.

---

# 3) Architecture (say and draw on board)

Simple diagram (you can draw):

```
Client -> API Gateway -> Leaderboard API Servers (scale out)
   |                                     |
   v                                     v
 Redis Cluster (ZSETs) <----> Event Broker (Kafka)
   ^                                     |
   |                                     v
   +---------- DBSyncService Consumers -> Persistent DB (Postgres/Cassandra/DynamoDB)
```

Sequence for AddScore:

1. Client -> API server
2. API server -> Redis `ZINCRBY leaderboard:global delta playerId`
3. API server (or Redis stream) -> produce `ScoreUpdate` event to Kafka
4. DBSyncService consumes events -> batch-updates DB
5. Reads (GetTopN/GetRank) served from Redis

---

# 4) Data model & APIs (whiteboard)

Data (Redis ZSET):

* Key: `leaderboard:global` (or `leaderboard:region:{region}` for sharding)
* Member: `playerId` (string)
* Score: numeric score (double/int depending)

Persistent DB schema (relational or wide-column):

```
player_scores (player_id PK, score bigint, last_updated timestamp)
```

API signatures:

```text
POST /score/update { playerId, delta }
GET  /leaderboard/top?n=100
GET  /leaderboard/rank?playerId=abc
```

Event schema (Kafka message):

```json
{
  "playerId": "p123",
  "delta": 50,
  "newScore": 1500,
  "timestamp": "2025-10-27T12:00:00Z"
}
```

---

# 5) Complexity & scaling bullets (say these)

* Redis ZINCRBY: O(log N) per update.
* ZREVRANGE for top N: O(log N + N).
* ZREVRANK (rank): O(log N).
* Kafka writes/reads: amortized O(1) for produce, consumer can batch.
* DBSyncService batching reduces DB write QPS.

Sharding:

* Partition leaderboards by region/gameMode to reduce single-key size or use Redis cluster automatic sharding by key.

---

# 6) Failure & consistency handling (talk through)

* If Redis write fails → return error to client.
* If Kafka produce fails → retry with backoff, persist to local buffer if necessary.
* If DBSync down → Kafka retains events until consumer restarts.
* If Redis crash before events are persisted → AOF/RDB can recover some; otherwise rebuild from persistent DB (DB might be slightly stale).
* Use idempotent updates in DBSync (e.g., write `score = max(score, event.newScore)` or upsert by player id with last_updated).

---

# 7) What to demo/code in interview (quick checklist)

* Show a minimal C++ `LeaderboardService` that:

  * Accepts update, calls Redis (pseudo-client) and publishes to Kafka.
* Show `DBSyncService` consumer code that batches events and does DB upserts.
* If asked to run locally, provide an in-memory fallback to simulate Redis + queue.

---

# 8) C++ Implementation Sketch

Below are runnable-style skeletons you can paste in the interview. They are **implementation sketches** — in production you’d use real clients (`hiredis` / `cpp_redis` for Redis, `librdkafka` for Kafka, and a DB client like `libpqxx` or a Cassandra client). For interview clarity I keep external details minimal and focus on interfaces, concurrency, batching, and error handling patterns.

---

### A. Common headers / types

```cpp
// types.h
#pragma once
#include <string>
#include <chrono>

struct ScoreUpdateEvent {
    std::string playerId;
    int64_t delta;
    int64_t newScore; // optional: may be filled by leader service
    std::chrono::system_clock::time_point timestamp;
};
```

---

### B. Redis wrapper (pseudo-code, replace with real client in production)

```cpp
// redis_client.h
#pragma once
#include <string>
#include <optional>

class RedisClient {
public:
    RedisClient(const std::string& host, int port) {
        // connect to Redis (use hiredis/cpp_redis)
    }

    // Atomically increment member in sorted set and return new score
    std::optional<int64_t> zincrby(const std::string& key, int64_t delta, const std::string& member) {
        // PSEUDO: call ZINCRBY and parse response
        // In production: handle errors, reconnect, retries
        // return new score on success, nullopt on failure
        return std::make_optional<int64_t>(/* newScore */ 0);
    }

    // Get top N members with scores (returns vector of pair<playerId, score>)
    // Implementation uses ZREVRANGE WITHSCORES
};
```

---

### C. Kafka publisher interface (pseudo)

```cpp
// event_publisher.h
#pragma once
#include "types.h"
#include <string>

class EventPublisher {
public:
    EventPublisher(const std::string& brokers, const std::string& topic) {
        // init librdkafka producer
    }

    // Non-blocking fire-and-forget with best-effort retry
    bool publish(const ScoreUpdateEvent& ev) {
        // serialize ev to JSON or protobuf and produce to kafka
        // return true if queued successfully
        return true;
    }
};
```

---

### D. LeaderboardService (main)

```cpp
// leaderboard_service.h
#pragma once
#include "redis_client.h"
#include "event_publisher.h"
#include "types.h"
#include <string>
#include <iostream>

class LeaderboardService {
public:
    LeaderboardService(RedisClient& rc, EventPublisher& ep, const std::string& leaderboardKey)
    : redis(rc), publisher(ep), key(leaderboardKey) {}

    // Called by HTTP handler or RPC
    bool addScore(const std::string& playerId, int64_t delta) {
        // 1) Update Redis
        auto newScoreOpt = redis.zincrby(key, delta, playerId);
        if(!newScoreOpt.has_value()) {
            // Redis failure => return error to client
            std::cerr << "Redis zincrby failed\n";
            return false;
        }
        int64_t newScore = newScoreOpt.value();

        // 2) Publish event to Kafka for persistence (write-behind)
        ScoreUpdateEvent ev{playerId, delta, newScore, std::chrono::system_clock::now()};
        bool ok = publisher.publish(ev);
        if(!ok) {
            // Log and optionally maintain local retry buffer
            std::cerr << "Failed to publish event, consider retrying\n";
        }

        return true; // immediate success to client
    }

    // getTopN and getRank would query redis zrevrange / zrevrank
private:
    RedisClient& redis;
    EventPublisher& publisher;
    std::string key;
};
```

Comments to make in interview: show how to wire this to HTTP handler (e.g., using pistache / boost::beast / cpp-httplib). Emphasize that the `addScore` returns quickly after Redis update and queuing.

---

### E. DBSyncService (consumer microservice): batch & idempotency

```cpp
// dbsync_service.h
#pragma once
#include "types.h"
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>

class DBSyncService {
public:
    DBSyncService(const std::string& brokers, const std::string& topic) {
        // init kafka consumer (librdkafka), subscribe
    }

    void run() {
        std::vector<ScoreUpdateEvent> batch;
        batch.reserve(1000);

        while(true) {
            // poll kafka for messages (blocking up to x ms)
            auto events = pollKafka(500); // pseudo call
            for(auto &ev : events) {
                // Optionally merge updates per playerId for this batch
                batch.push_back(ev);
                if(batch.size() >= 1000) {
                    flushBatch(batch);
                }
            }

            // periodic flush by time
            if(!batch.empty() && timeSinceLastFlush() >= std::chrono::seconds(1)) {
                flushBatch(batch);
            }
        }
    }

private:
    std::vector<ScoreUpdateEvent> pollKafka(int timeoutMs) {
        // return vector parsed from messages
        return {};
    }

    void flushBatch(std::vector<ScoreUpdateEvent>& batch) {
        // 1) Merge by playerId to reduce writes
        std::unordered_map<std::string, int64_t> merged;
        for(const auto& ev : batch) {
            merged[ev.playerId] += ev.delta; // sum deltas; or keep newScore
        }

        // 2) Do batch DB upserts (pseudocode)
        // In SQL you can execute multi-row upsert statements or use COPY
        // In NoSQL, batch mutates.
        try {
            // db.beginTransaction();
            // for(auto &kv : merged) db.upsertScore(kv.first, kv.second);
            // db.commit();
        } catch(...) {
            // On failure: log, push back to retry (or rely on kafka offset)
        }

        batch.clear();
    }

    long timeSinceLastFlush() {
        // pseudo
        return 2;
    }
};
```

Important interview points:

* Merge updates per player inside batch to reduce DB writes (summing deltas or using the final newScore for idempotency).
* Use DB upsert semantics: `INSERT ... ON CONFLICT (player_id) DO UPDATE SET score = EXCLUDED.score, last_updated = NOW();`
* Commit/transaction & retries.

---

### F. In-memory local fallback for demo (single process)

If interviewer asks you to code-run on laptop during interview, show an in-memory implementation of `LeaderboardService` using `std::map<int64_t, std::unordered_set<string>>` or `std::multimap` and an internal queue. This demonstrates correct logic without external dependencies.

---

# 9) Concurrency details (speak)

* Redis operations are atomic for single commands like `ZINCRBY` — no application-side locks needed for score updates.
* In-app concurrency: the API server must use thread-safe clients for Redis and the Kafka producer (most clients are thread-safe when used correctly).
* DBSync batching: create multiple consumer instances and use Kafka consumer groups to scale consumers.

---

# 10) Idempotency & Exactly-once concerns (say this clearly)

* Kafka provides at-least-once semantics by default; consumers should be idempotent.
* Use **upsert by player_id** and store `last_updated` timestamp to avoid applying older events after newer ones.
* Optionally include an event sequence number per player (incremented by service) to avoid reordering issues.

---

# 11) Monitoring & SLOs to mention

* Monitor Redis: memory usage, latency, eviction rate.
* Monitor Kafka: consumer lag, broker health.
* Monitor DBSync: batch sizes, DB write latency, replication lag.
* SLOs: 99th percentile latency for `addScore` (should be Redis latency + network), availability (99.99% target).
* Alerts on consumer-lag > threshold.

---

# 12) Testing & Demo plan

* Unit tests: LeaderboardService logic with mocked Redis & publisher.
* Integration test: Local Redis + Kafka (docker compose) to test end-to-end.
* Load test: JMeter / Locust to simulate millions of score updates; measure throughput & consumer lag.
* Chaos test: Kill DBSync, kill Redis, validate recovery and data rebuild from DB.

---

# 13) Things to call out in interview as tradeoffs

* Write-behind reduces latency but risks short-term loss. Mitigate via AOF + Kafka durable events.
* Redis memory cost vs disk-based DB cost: shard or tier leaderboards (hot vs cold).
* Choice of persistent DB: Relational easier for small scale, Cassandra/DynamoDB for multi-region high write workloads.
* Alternative: Instead of Kafka, consider Redis Streams for a simpler Redis-only stack.

---

# 14) Example "what I would write on board" summary

* Draw boxes for API servers, Redis cluster, Kafka, DBSync consumers, DB.
* Show `AddScore` flow with arrows and event schema.
* List commands: `ZINCRBY`, `ZREVRANGE`, `ZREVRANK`, Kafka `produce/consume`.

---

# 15) Closing lines to say to interviewer

“I can implement the above C++ code using hiredis and librdkafka; the core logic fits in under 300 lines, and we’ll unit-test the service with mocks. For production we’d add TLS, client pools, retry & backoff, metrics, and autoscaling for both API servers and DBSync consumers. Do you want me to show a runnable version using an in-memory mock, or go deeper into the Kafka consumer/producer code?”

---

# 16) Extra: quick checklist of libraries & commands to mention (shows real-world knowledge)

* Redis client libs: `hiredis`, `cpp_redis` or `redis-plus-plus`.
* Kafka producer/consumer: `librdkafka` (and `cppkafka` wrapper).
* DB client: `libpqxx` for PostgreSQL; DataStax C++ driver for Cassandra.
* HTTP frameworks: `Boost.Beast`, `pistache`, `cpp-httplib`.
* Serialization: `protobuf` or `nlohmann::json`.

---

# 17) Final small code snippet you can paste in interview (single-file in-memory demo)

```cpp
// tiny_demo.cpp - in-memory single-threaded demo for interview
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <set>

class SimpleLeaderboard {
    // map score -> set of players (descending order)
    std::map<long long, std::set<std::string>, std::greater<>> scoreMap;
    std::unordered_map<std::string, long long> playerToScore;
public:
    void addScore(const std::string& player, long long delta) {
        long long old = 0;
        auto it = playerToScore.find(player);
        if(it != playerToScore.end()) old = it->second;
        long long ns = old + delta;
        if(old != 0) {
            auto &s = scoreMap[old];
            s.erase(player);
            if(s.empty()) scoreMap.erase(old);
        }
        scoreMap[ns].insert(player);
        playerToScore[player] = ns;
    }
    std::vector<std::pair<std::string,long long>> topN(int n) {
        std::vector<std::pair<std::string,long long>> res;
        for(auto &p : scoreMap) {
            for(auto &player : p.second) {
                res.push_back({player, p.first});
                if((int)res.size() == n) return res;
            }
        }
        return res;
    }
    int rankOf(const std::string& player) {
        auto it = playerToScore.find(player);
        if(it == playerToScore.end()) return -1;
        long long s = it->second;
        int r = 1;
        for(auto &p : scoreMap) {
            if(p.first == s) break;
            r += (int)p.second.size();
        }
        return r;
    }
};

int main(){
    SimpleLeaderboard lb;
    lb.addScore("alice", 50);
    lb.addScore("bob", 30);
    lb.addScore("alice", 25);
    auto top = lb.topN(10);
    for(auto &t : top) std::cout<<t.first<<":"<<t.second<<"\n";
    std::cout<<"alice rank: "<<lb.rankOf("alice")<<"\n";
    return 0;
}
```

Use this in the interview to show you can code and explain tradeoffs: it’s correct, simple, and demonstrates the leader logic.

---

If you want, I can now:

* Convert the pseudo-Redis / Kafka code into actual compilable C++ using `redis-plus-plus` and `librdkafka` (I’ll include `CMakeLists` and example connection code).
* Or produce a one-page answer sheet / whiteboard image you can print for interviews.

Which of those would you like next?
