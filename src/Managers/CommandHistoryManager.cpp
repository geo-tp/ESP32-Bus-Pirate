#include "CommandHistoryManager.h"

/*
Add
*/
void CommandHistoryManager::add(const std::string& line) {
    if (!line.empty() && (history.empty() || history.back() != line)) {
        history.push_back(line);

        if (history.size() > maxHistory) {
            history.erase(history.begin());
        }
    }
    index = history.size();
}

/*
Up History
*/
const std::string& CommandHistoryManager::up() {
    if (index > 0) {
        --index;
    }
    currentLine = (index < history.size()) ? history[index] : "";
    return currentLine;
}

/*
Down History
*/
const std::string& CommandHistoryManager::down() {
    if (index < history.size() - 1) {
        ++index;
    } else {
        index = history.size();
        currentLine.clear();
    }
    currentLine = (index < history.size()) ? history[index] : "";
    return currentLine;
}

/*
Auto complete
*/
const std::string& CommandHistoryManager::autocomplete(const std::string& prefix) {
    currentLine.clear();

    if (prefix.empty() || history.empty()) {
        return currentLine;
    }

    // newest -> oldest
    for (size_t i = history.size(); i-- > 0;) {
        const std::string& h = history[i];

        if (h.size() >= prefix.size() && h.compare(0, prefix.size(), prefix) == 0) {
            currentLine = h;
            index = i;
            return currentLine;
        }
    }

    // no match
    return currentLine;
}

/*
Reset History
*/
void CommandHistoryManager::reset() {
    index = history.size();
}

/*
Size
*/
size_t CommandHistoryManager::size() const {
    return history.size();
}
