#include "TerminalCommandTransformer.h"
#include "Data/AutoCompleteWords.h"
#include "Enums/ModeEnum.h"
#include <sstream>
#include <cstring>

TerminalCommand TerminalCommandTransformer::transform(const std::string& raw) const {
    std::string normalized = normalizeRaw(raw);
    std::istringstream iss(normalized);

    std::string root, subcommand, args;
    iss >> root >> subcommand;
    std::getline(iss, args);

    if (!args.empty() && args[0] == ' ') {
        args.erase(0, 1);
    }

    TerminalCommand cmd;
    cmd.setRoot(root);
    cmd.setSubcommand(subcommand);
    cmd.setArgs(args);

    autoCorrectRoot(cmd);
    autoCorrectSubCommand(cmd);

    return cmd;
}

std::string TerminalCommandTransformer::normalizeRaw(const std::string& raw) const {
    // trim global
    size_t start = raw.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = raw.find_last_not_of(" \t\r\n");
    std::string s = raw.substr(start, end - start + 1);

    // extract root
    std::istringstream iss(s);
    std::string root;
    iss >> root;

    // lowercase root
    for (char& c : root) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // get rest of line 
    std::string rest;
    std::getline(iss, rest); // may start with spaces

    // avoid double spaces between root and subcommand
    if (!rest.empty() && rest[0] == ' ') {
        size_t firstNonSpace = rest.find_first_not_of(' ');
        if (firstNonSpace != std::string::npos) {
            rest.erase(0, firstNonSpace - 1);
        }
    }

    return root + rest;
}

void TerminalCommandTransformer::autoCorrectRoot(TerminalCommand& cmd) const {
    std::string root = cmd.getRoot();
    if (root.empty()) return;

    // Avoid correcting very short root
    if (root.size() < 3) return;

    // Exact match?
    for (int i = 0; autoCompleteWords[i] != nullptr; ++i) {
        const char* w = autoCompleteWords[i];
        if (std::strchr(w, ' ') != nullptr) continue; // skip "mode xxx"
        if (root == w) return;
    }

    const char* best = nullptr;
    int bestScore = 999;
    int bestCount = 0;

    for (int i = 0; autoCompleteWords[i] != nullptr; ++i) {
        const char* w = autoCompleteWords[i];
        if (std::strchr(w, ' ') != nullptr) continue;

        int score = scoreTightEditDistance(root, w);
        if (score >= 999) continue;

        if (score < bestScore) {
            bestScore = score;
            best = w;
            bestCount = 1;
            if (bestScore == 0) break;
        } else if (score == bestScore) {
            bestCount++;
        }
    }

    // Accept only very tight corrections
    if (best && bestScore <= 1 && bestCount == 1) {
        cmd.setRoot(best);
    }
}

void TerminalCommandTransformer::autoCorrectSubCommand(TerminalCommand& cmd) const {
    // Only for "mode" / "m"
    std::string root = cmd.getRoot();
    for (char& c : root) c = (char)std::tolower((unsigned char)c);

    if (root == "m") {
        cmd.setRoot("mode");
        root = "mode";
    }
    if (root != "mode") return;

    std::string sub = cmd.getSubcommand();
    if (sub.empty()) return;

    // trim
    size_t start = sub.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return;
    size_t end = sub.find_last_not_of(" \t\r\n");
    sub = sub.substr(start, end - start + 1);

    // lowercase subcmd
    for (char& c : sub) c = (char)std::tolower((unsigned char)c);

    // aliases for protocols
    struct Alias { const char* from; const char* to; };
    static const Alias aliases[] = {
        {"onewire",   "1wire"},
        {"1w",        "1wire"},
        {"twowire",   "2wire"},
        {"2w",        "2wire"},
        {"threewire", "3wire"},
        {"3w",        "3wire"},
        {"i2",        "i2c"},
        {"serial",    "uart"},
        {"eth",       "ethernet"},
        {"bt",        "bluetooth"},
        {"ble",       "bluetooth"},
        {"ir",        "infrared"},
        {"sub",       "subghz"},
        {"rf",        "rf24"},
        {"nfc",       "rfid"},

        {nullptr, nullptr}
    };

    for (int i = 0; aliases[i].from; ++i) {
        if (sub == aliases[i].from) {
            cmd.setSubcommand(aliases[i].to);
            return;
        }
    }

    // get protocol names
    auto actions = ModeEnumMapper::getProtocols();
    auto actionsStr = ModeEnumMapper::getProtocolNames(actions);

    // exact match ?
    for (const auto& s : actionsStr) {
        std::string cand = s;
        for (char& c : cand) c = (char)std::tolower((unsigned char)c);
        if (sub == cand) return;
    }

    if (sub.size() < 2) return;

    int bestIdx = -1;
    int bestScore = 999;
    int bestCount = 0;

    for (int i = 0; i < (int)actionsStr.size(); ++i) {
        std::string cand = actionsStr[(size_t)i];
        for (char& c : cand) c = (char)std::tolower((unsigned char)c);

        int score = scoreTightEditDistance(sub, cand.c_str());
        if (score >= 999) continue;

        if (score < bestScore) {
            bestScore = score;
            bestIdx = i;
            bestCount = 1;
            if (bestScore == 0) break;
        } else if (score == bestScore) {
            bestCount++;
        }
    }

    // Apply only small correction (score==1) and non ambiguous
    if (bestIdx >= 0 && bestScore == 1 && bestCount == 1) {
        cmd.setSubcommand(actionsStr[(size_t)bestIdx]);
    }
}

// 0 = exact, 1 = "tight" correction, 999 = not close
int TerminalCommandTransformer::scoreTightEditDistance(const std::string& a, const char* b) const {
    if (!b) return 999;
    const int al = (int)a.size();
    const int bl = (int)std::strlen(b);
    if (a == b) return 0;

    int diff = al - bl; if (diff < 0) diff = -diff;
    if (diff > 1) return 999;

    // Same length: 1 mismatch OR adjacent transposition
    if (al == bl) {
        int mism = 0;
        int first = -1, second = -1;

        for (int i = 0; i < al; ++i) {
            if (a[(size_t)i] != b[i]) {
                if (mism == 0) first = i;
                else second = i;
                mism++;
                if (mism > 2) break;
            }
        }

        if (mism == 1) return 1;

        if (mism == 2 && second == first + 1) {
            if (a[(size_t)first] == b[second] && a[(size_t)second] == b[first]) {
                return 1; // transposition
            }
        }
        return 999;
    }

    // Insertion/deletion: allow skipping 1 char in the longer string
    // Determine which is shorter without constructing std::string
    const bool aIsShort = (al < bl);
    int i = 0, j = 0, skips = 0;

    while ((aIsShort ? (i < al) : (i < bl)) && (aIsShort ? (j < bl) : (j < al))) {
        const char ca = aIsShort ? a[(size_t)i] : b[i];
        const char cb = aIsShort ? b[j] : a[(size_t)j];

        if (ca == cb) { i++; j++; }
        else { skips++; j++; if (skips > 1) return 999; }
    }

    return 1;
}
