
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	Last revision 01/04/2020
 *
 */
#include <fs.h>
#include <dcache.h>
#include <linux/list.h>
#include <time.h>
#include <namei.h>



#ifdef __LITTLE_ENDIAN
 #define HASH_LEN_DECLARE u32 hash; u32 len
 #define bytemask_from_count(cnt)	(~(~0ul << (cnt)*8))
#else
 #define HASH_LEN_DECLARE u32 len; u32 hash
 #define bytemask_from_count(cnt)	(~(~0ul >> (cnt)*8))
#endif

struct super_block{
    dev_t s_dev;
    unsigned long s_blocksize;
    struct file_system_type *s_type;
    struct super_operations *s_op;
    struct dentry *s_root;
};

struct dentry{
    struct inode *d_inode;
    struct dentry *d_parent;
    struct qstr d_name;
    struct dentry_operations *d_op;
    struct super_block *d_sb;
    struct list_head d_subdirs;
};

struct dentry_operations{
    int (*d_revalidate) (struct dentry *, int);
    int (*d_hash) (struct dentry *, struct qstr *);
    int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
    int (*d_delete) (struct dentry *);
    void (*d_release) (struct dentry *);
    void (*d_iput) (struct dentry *, struct inode *);
};

struct qstr {
    union {
        struct {
            HASH_LEN_DECLARE;
        };
        u64 hash_len;
    };
    const unsigned char *name;
};

struct inode{
    unsigned long i_ino;
    umode_t i_mode;
    uid_t i_uid;
    gid_t i_gid;
    kdev_t i_rdev;
    loff_t i_size;
    struct timespec i_atime;
    struct timespec i_ctime;
    struct timespec i_mtime;
    struct super_block *i_sb;
    struct inode_operations *i_op;
    struct address_space *i_mapping;
    struct list_head i_dentry;
};

struct inode_operations{
    int (*create) (struct inode *, struct dentry *, int);
    int (*unlink) (struct inode *, struct dentry *);
    int (*mkdir) (struct inode *, struct dentry *, int);
    int (*rmdir) (struct inode *, struct dentry *);
    int (*mknod) (struct inode *, struct dentry *, int, dev_t);
    int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
    void (*truncate) (struct inode *);
    struct dentry * (*lookup) (struct inode *, struct dentry *);
    int (*permission) (struct inode *, int);
    int (*setattr) (struct dentry *, struct iattr *);
    int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
    int (*setxattr) (struct dentry *, const char *, const void *, size_t, int);
    ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
    ssize_t (*listxattr) (struct dentry *, char *, size_t);
    int (*removexattr) (struct dentry *, const char *);
    int (*link) (struct dentry *, struct inode *, struct dentry *);
    int (*symlink) (struct inode *, struct dentry *, const char *);
    int (*readlink) (struct dentry *, char *, int);
    int (*follow_link) (struct dentry *, struct nameidata *);
};

struct lista{
    struct inodo content;////////////////////////////////////////////////////////////////////////////
    struct lista *next;
    struct lista *prev;
};




