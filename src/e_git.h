#ifndef __E_GIT_H__
#define __E_GIT_H__

enum GitLineStatus {
    GIT_CLEAN    = 0,
    GIT_ADDED    = 1,
    GIT_MODIFIED = 2,
    GIT_DELETED  = 3
};

#define GIT_GUTTER_WIDTH 1

#endif // __E_GIT_H__
