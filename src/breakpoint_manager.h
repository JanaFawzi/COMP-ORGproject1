#ifndef BREAKPOINT_MANAGER_H
#define BREAKPOINT_MANAGER_H

class BreakpointManager {
public:
    static constexpr int MAX_BREAKPOINTS = 64;

    BreakpointManager() {
        reset();
    }

    void reset() {
        count = 0;

        for (int i = 0; i < MAX_BREAKPOINTS; i++) {
            addresses[i] = 0;
        }
    }

    static bool isValidBreakpointAddress(unsigned short address) {
        if ((address & 1) != 0) {
            return false;
        }

        return true;
    }

    bool setBreakpoint(unsigned short address) {
        if (!isValidBreakpointAddress(address)) {
            return false;
        }

        if (hasBreakpoint(address)) {
            return true;
        }

        if (count >= MAX_BREAKPOINTS) {
            return false;
        }

        addresses[count] = address;
        count++;

        return true;
    }

    bool clearBreakpoint(unsigned short address) {
        int index = findBreakpointIndex(address);

        if (index < 0) {
            return false;
        }

        for (int i = index; i < count - 1; i++) {
            addresses[i] = addresses[i + 1];
        }

        addresses[count - 1] = 0;
        count--;

        return true;
    }

    bool toggleBreakpoint(unsigned short address) {
        if (hasBreakpoint(address)) {
            return clearBreakpoint(address);
        }

        return setBreakpoint(address);
    }

    bool hasBreakpoint(unsigned short address) {
        if (!isValidBreakpointAddress(address)) {
            return false;
        }

        if (findBreakpointIndex(address) >= 0) {
            return true;
        }

        return false;
    }

    int getBreakpointCount() {
        return count;
    }

    unsigned short getBreakpointAtIndex(int index) {
        if (index < 0 || index >= count) {
            return 0;
        }

        return addresses[index];
    }

private:
    unsigned short addresses[MAX_BREAKPOINTS];
    int count;

    int findBreakpointIndex(unsigned short address) {
        for (int i = 0; i < count; i++) {
            if (addresses[i] == address) {
                return i;
            }
        }

        return -1;
    }
};

#endif
