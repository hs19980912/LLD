// minimal_scheduler.cpp
// g++ -std=c++17 minimal_scheduler.cpp -o minimal_scheduler

#include <bits/stdc++.h>
using namespace std;

// ---------- Models ----------
struct Meeting {
    int id;
    int organizerId;
    int start; // inclusive
    int end;   // exclusive
    vector<int> participants; // includes organizer
};

class Scheduler {
    int nextMeetingId = 1;
    
    unordered_map<int, string> users;   // userId -> userName (optional, but helpful)
    
    unordered_map<int, vector<int>> calendars;  // userId -> list of meeting ids (keeps meetings per calendar)
    
    unordered_map<int, Meeting> meetings;   // meetingId -> Meeting

public:
    bool addUser(int userId, const string &name = "") {
        if (users.count(userId)) return false;
        users[userId] = name;
        calendars[userId] = {};
        return true;
    }

    // Utility: check overlap of two intervals [s1,e1) and [s2,e2)
    static bool overlap(int s1, int e1, int s2, int e2) {
        return !(e1 <= s2 || e2 <= s1);
    }

    // Check if user is free in [start, end)
    bool isUserFree(int userId, int start, int end) {
        if (!users.count(userId)) return false; // user not found -> treat as not free
        for (int mid : calendars[userId]) {
            const Meeting &m = meetings[mid];
            // treat any meeting in calendar as busy (minimal version: no accept/decline)
            if (overlap(start, end, m.start, m.end)) return false;
        }
        return true;
    }

    // Create a meeting if all participants are free
    // participants vector should contain participant ids (organizer may or may not be included)
    // returns meeting id or -1 on conflict/failure
    int createMeeting(int organizerId, int start, int end, vector<int> participants) {
        
        // ensure organizer is in participants
        if (find(participants.begin(), participants.end(), organizerId) == participants.end())
            participants.push_back(organizerId);

        // check user existence and availability
        for (int uid : participants) {
            if (!users.count(uid)) return -1;
            if (!isUserFree(uid, start, end)) return -1; // conflict
        }

        int mid = nextMeetingId++;
        Meeting m{mid, organizerId, start, end, participants};
        meetings[mid] = m;
        for (int uid : participants) calendars[uid].push_back(mid);

        // keep each user's calendar sorted by meeting start (simple insertion sort)
        for (int uid : participants) {
            auto &vec = calendars[uid];
            sort(vec.begin(), vec.end(), [&](int a, int b){
                return meetings[a].start < meetings[b].start;
            });
        }
        return mid;
    }

    // Return meetings for a user (sorted by start)
    vector<Meeting> getMeetings(int userId) {
        vector<Meeting> out;
        if (!users.count(userId)) return out;
        for (int mid : calendars[userId]) out.push_back(meetings[mid]);
        sort(out.begin(), out.end(), [](const Meeting &a, const Meeting &b){
            return a.start < b.start;
        });
        return out;
    }

    // Find earliest common slot start time (integer) in [searchStart, searchEnd) where all users are free for 'duration'.
    // returns start time or -1 if no slot.
    int findCommonSlot(const vector<int> &userIds, int duration, int searchStart, int searchEnd, int step = 1) {
        if (duration <= 0 || searchStart < 0 || searchEnd <= searchStart) return -1;
        for (int t = searchStart; t + duration <= searchEnd; t += step) {
            bool ok = true;
            for (int uid : userIds) {
                if (!isUserFree(uid, t, t + duration)) { ok = false; break; }
            }
            if (ok) return t;
        }
        return -1;
    }

    // Optional helper: cancel meeting (only organizer can cancel)
    bool cancelMeeting(int meetingId, int requesterId) {
        if (!meetings.count(meetingId)) return false;
        const Meeting &m = meetings[meetingId];
        if (m.organizerId != requesterId) return false;
        // remove meeting from calendars
        for (int uid : m.participants) {
            auto &vec = calendars[uid];
            vec.erase(std::remove(vec.begin(), vec.end(), meetingId), vec.end());
        }
        meetings.erase(meetingId);
        return true;
    }
};

// ---------- Example usage ----------
int main() {
    Scheduler s;
    s.addUser(1, "Alice");
    s.addUser(2, "Bob");
    s.addUser(3, "Carol");

    // Time units: integer (e.g., minutes). We'll use hours represented as integers for simplicity:
    // 9 -> 9:00, 10 -> 10:00, etc.
    int m1 = s.createMeeting(1, 9, 10, {2}); // Alice organizes 9-10 with Bob
    cout << "Meeting 1 id: " << m1 << "\n";

    // Conflict: Bob is busy 9-10, so this should fail
    int m2 = s.createMeeting(3, 9, 10, {2}); // Carol wants 9-10 with Bob
    cout << "Meeting 2 id (expect -1): " << m2 << "\n";

    // Find slot for Alice and Carol for 1 hour between 8..18
    int slot = s.findCommonSlot({1,3}, 1, 8, 18);
    cout << "Found slot (start hour) for Alice+Carol: " << slot << "\n";

    // Print Alice's meetings
    auto am = s.getMeetings(1);
    cout << "Alice meetings:\n";
    for (auto &m : am) {
        cout << "  id=" << m.id << " [" << m.start << "," << m.end << ")\n";
    }

    // Cancel meeting
    bool cancelled = s.cancelMeeting(m1, 1);
    cout << "Cancelled meeting " << m1 << ": " << (cancelled ? "yes" : "no") << "\n";

    return 0;
}
