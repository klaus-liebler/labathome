#pragma once

enum class LabAtHomeErrorCode:int
{
    OK = 0,
    GENERIC_ERROR = 1,
    QUEUE_OVERLOAD = 2,
    NONE_AVAILABLE=3,
    INDEX_OUT_OF_BOUNDS=4,
    INVALID_NEW_FBD=5,
    NOT_YET_IMPLEMENTED = 6,
    TEMPORARYLY_NOT_AVAILABLE = 7,
    FILE_SYSTEM_ERROR=8,
    INCOMPATIBLE_VERSION=9,
    EMERGENCY_SHUTDOWN=10,
};