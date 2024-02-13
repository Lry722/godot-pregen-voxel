#define CONSTANT_STRING(name, str) static const char *name = str;

namespace lry
{
    namespace config
    {
        namespace filename
        {
            CONSTANT_STRING(kConfig, "config.yml")
        } // namespace filename
        
    } // namespace config
    
    namespace db
    {
        namespace filename
        {
            CONSTANT_STRING(kDatabaseEnv, "world.db")
            CONSTANT_STRING(kTerrainDB, "terrain")
        }
        static const unsigned int kPermission = 0664;
    }

}