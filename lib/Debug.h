#ifndef DEBUG_H
#define DEBUG_H

// @TODO: do this within the project settings
#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
#define DEBUG(stream) { std::cerr << /*__FILE__ << ":" << __LINE__ << ": " <<*/ stream << std::endl; }
#else
#define DEBUG(stream)
#endif

#endif // DEBUG_H
