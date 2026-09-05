#include "fte.h"
#include "e_git.h"

void EBuffer::FreeGitStatus() {
    if (GitStatus) {
        free(GitStatus);
        GitStatus = NULL;
    }
    GitStatusCount = 0;
}

int EBuffer::UpdateGitStatus() {
    FreeGitStatus();

    if (!FileName || FileName[0] == '\0')
        return 0;

    char dir[MAXPATH];
    char name[MAXPATH];
    if (JustDirectory(FileName, dir, sizeof(dir)) == -1)
        return 0;
    if (JustFileName(FileName, name, sizeof(name)) == -1)
        return 0;

    // Allocate status array matching RCount
    if (RCount <= 0) return 0;
    GitStatus = (char *)calloc(RCount, sizeof(char));
    if (!GitStatus) return 0;
    GitStatusCount = RCount;

    // Command: git -C <dir> diff --no-color -U0 HEAD -- <file>
    char cmd[MAXPATH * 2 + 64];
    snprintf(cmd, sizeof(cmd), "git -C \"%s\" diff --no-color -U0 HEAD -- \"%s\" 2>/dev/null", dir, name);

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        FreeGitStatus();
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] != '@' || line[1] != '@')
            continue;

        // Parse: @@ -<old_start>[,<old_count>] +<new_start>[,<new_count>] @@
        int old_start = 0, old_count = 1;
        int new_start = 0, new_count = 1;

        char *plus = strchr(line, '+');
        if (!plus) continue;

        // Parse old
        char *minus = strchr(line, '-');
        if (minus && minus < plus) {
            old_start = atoi(minus + 1);
            char *comma = strchr(minus, ',');
            if (comma && comma < plus)
                old_count = atoi(comma + 1);
        }

        // Parse new
        new_start = atoi(plus + 1);
        char *comma = strchr(plus, ',');
        char *at_end = strstr(plus, "@@");
        if (comma && (!at_end || comma < at_end))
            new_count = atoi(comma + 1);

        // Map to 0-based line indices
        if (old_count == 0 && new_count > 0) {
            // Addition
            for (int i = 0; i < new_count; i++) {
                int row = (new_start - 1) + i;
                if (row >= 0 && row < GitStatusCount)
                    GitStatus[row] = GIT_ADDED;
            }
        } else if (new_count == 0 && old_count > 0) {
            // Deletion
            int row = new_start - 1;
            if (row < 0) row = 0;
            if (row >= 0 && row < GitStatusCount)
                GitStatus[row] = GIT_DELETED;
        } else {
            // Modification
            for (int i = 0; i < new_count; i++) {
                int row = (new_start - 1) + i;
                if (row >= 0 && row < GitStatusCount)
                    GitStatus[row] = GIT_MODIFIED;
            }
        }
    }
    pclose(fp);
    FullRedraw();
    return 1;
}

char EBuffer::GetGitLineStatus(int Row) {
    if (!GitStatus || Row < 0 || Row >= GitStatusCount)
        return GIT_CLEAN;
    return GitStatus[Row];
}
