#ifndef _Controller_h_
#define _Controller_h_

class Nit;
class Sdt;
class Bat;
class Ts;

template<typename Table> class DataWrapper;

class Controller
{
public:
    void Start() const;
    void NitTrigger(const DataWrapper<Nit>&);
    void SdtTrigger(const DataWrapper<Sdt>&);
    void BatTrigger(const DataWrapper<Bat>&);

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

private:
    Controller();
    std::fstream nitTsFile;
    std::fstream sdtTsFile;
    std::fstream batTsFile;
    std::shared_ptr<Ts> nitTs;
    std::shared_ptr<Ts> sdtTs;
    std::shared_ptr<Ts> batTs;
    std::list<std::shared_ptr<DataWrapper<Nit>>> nitWrappers; 
    std::list<std::shared_ptr<DataWrapper<Sdt>>> sdtWrappers; 
    std::list<std::shared_ptr<DataWrapper<Bat>>> batWrappers; 
};

#endif