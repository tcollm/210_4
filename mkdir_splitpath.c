#include "types.h"

extern struct NODE *root;
extern struct NODE *cwd;

struct NODE *findNode(struct NODE *current, const char *target)
{
    if (current == NULL)
    {
        return NULL;
    }

    // target is found
    if (strcmp(current->name, target) == 0)
    {
        return current;
    }

    // recursively search children
    struct NODE *foundInChild = findNode(current->childPtr, target);
    if (foundInChild != NULL)
    {
        return foundInChild;
    }

    // recursively search siblings
    return findNode(current->siblingPtr, target);
}

struct NODE *splitPath(char *pathName, char *baseName, char *dirName)
{
    int i = 0;
    int lastIndex = -1;

    while (pathName[i] != '\0')
    {
        // find the last / so that dirName can be everything before it
        // and baseName can be the dir after it
        if (pathName[i] == '/')
        {
            lastIndex = i;
            // return; DO NOT RETURN - will cause the loop to not work
        }
        i++;
    }

    // assign baseName and dirName
    // if there are no '/' then assign dirName to the cwd
    // and assign the pathName to baseName
    if (lastIndex == -1)
    {
        // dirName[0] = '/';
        strcpy(dirName, cwd->name);
        // dirName[1] = '\0';
        strcpy(baseName, pathName);
    }
    else
    {
        strncpy(dirName, pathName, lastIndex);
        dirName[lastIndex] = '\0';
        strcpy(baseName, pathName + lastIndex + 1);
    }

    // start at root if path starts with '/'
    struct NODE *start = (dirName[0] == '/') ? root : cwd;

    // tokenize path and then return dirName as the parent dir to baseName
    char *dirNameCpy = (char *)malloc(sizeof(char) * 256);
    strcpy(dirNameCpy, dirName);
    char *token;
    token = strtok(dirName, "/");
    while (token != NULL)
    {
        // printf("token: %s\n", token);
        strcpy(dirName, token);
        token = strtok(NULL, "/");
    }
    // printf("dirName: %s\n", dirName);

    // find the dir node using its name
    struct NODE *dirNode = findNode(start, dirName);

    // return a node
    if (dirNode && dirNode->fileType == 'D')
    {
        return dirNode;
    }
    else
    {
        // if dirName is not a directory then return ERROR
        // printf("ERROR: %s does not exist\n", dirName);
        return NULL;
    }

    // set r to the root or cwd if there is no '/'
    // while there are tokens (/'s)
    // set r = r->child
    // while r != NULL
    // r = r->child
    // r = r->sibling
    // return r
}

void createNode(char *baseName, struct NODE *parentDir)
{
    struct NODE *newNode = (struct NODE *)malloc(sizeof(struct NODE));

    if (!newNode)
    {
        printf("MKDIR ERROR: Memory allocation failed\n");
        return;
    }

    strcpy(newNode->name, baseName);
    newNode->fileType = 'D';
    newNode->childPtr = NULL;
    newNode->siblingPtr = NULL;
    newNode->parentPtr = parentDir;

    // attach to parent
    // parent's child is empty
    if (parentDir->childPtr == NULL)
    {
        parentDir->childPtr = newNode;
    }
    else
    {
        struct NODE *child = parentDir->childPtr;
        // parent's child is not empty, search for next empty sibling
        while (child->siblingPtr != NULL)
        {
            child = child->siblingPtr;
        }
        child->siblingPtr = newNode;
    }
}

struct NODE *findNodeInChildren(struct NODE *current, const char *target)
{
    // check self and then iterate to child (fixes sibling check error)
    if (strcmp(current->name, target) == 0)
    {
        return current;
    }
    current = current->childPtr;

    // This function specifically checks only the children of the provided current node
    while (current != NULL)
    {
        if (strcmp(current->name, target) == 0)
        {
            return current; // Return if the target is found
        }
        current = current->siblingPtr; // Move to the next sibling
    }
    return NULL; // Not found
}

void mkdir(char pathName[])
{
    // Check if pathName is empty or contains only whitespace characters
    int isEmptyOrWhitespace = 1;
    for (int i = 0; pathName[i] != '\0'; i++)
    {
        if (pathName[i] != ' ' && pathName[i] != '\t' && pathName[i] != '/')
        { // Include other whitespace characters as needed
            isEmptyOrWhitespace = 0;
            break;
        }
    }

    // check if pathName is actually provided
    if (isEmptyOrWhitespace)
    {
        printf("MKDIR ERROR: no path provided\n");
        return;
    }

    char *baseName = (char *)malloc(sizeof(char) * 256);
    char *dirName = (char *)malloc(sizeof(char) * 256);

    struct NODE *dirNode = splitPath(pathName, baseName, dirName);

    struct NODE *startDir = (pathName[0] == '/') ? root : cwd;
    char *startPathName = (char *)malloc(sizeof(char) * 256);

    // set startPathName to beginning of pathName
    int i = 0;
    while (pathName[i] != '/' && pathName[i] != '\0')
    {
        startPathName[i] = pathName[i];
        i++;
    }
    startPathName[i] = '\0'; // Null-terminate the string

    // if startPathName is not '/'
    // then check every child of the startDir to see if startPath exists
    if (strcmp(startPathName, baseName) != 0)
    {
        struct NODE *child = startDir->childPtr;
        int found = 0;
        while (child != NULL)
        {
            if (strcmp(child->name, startPathName) == 0)
            {
                found = 1;
                break;
            }
            child = child->siblingPtr;
        }

        if (!found)
        {
            printf("ERROR: directory %s does not exist\n", dirName);
            free(baseName);
            free(dirName);
            free(startPathName);
            return;
        }
    }

    // if (findNodeInChildren(startDir, dirName) == NULL)
    // {
    //     printf("ERROR: directory %s does not exist\n", dirName);
    //     free(baseName);
    //     free(dirName);
    //     return;
    // }

    // // ERROR: need to fix find node so that c/d does not work when in root:
    // if (findNodeInChildren(cwd, dirName) == NULL)
    // {
    //     printf("ERROR: directory %s does not exist\n", dirName);
    //     return;
    // }

    if (dirNode == NULL)
    {
        printf("MKDIR ERROR: Parent directory %s does not exist\n", dirName);
        free(baseName);
        free(dirName);
        return;
    }

    struct NODE *check = findNodeInChildren(dirNode, baseName);
    // struct NODE *check = findNode(dirNode, baseName);
    if (check != NULL)
    {
        printf("MKDIR ERROR: directory %s already exists\n", baseName);
        free(baseName);
        free(dirName);
        return;
    }

    createNode(baseName, dirNode); // Create the new directory node

    printf("MKDIR SUCCESS: node %s successfully created\n", pathName);

    free(baseName);
    free(dirName);
}
