#ifndef _Controller_h_
#define _Controller_h_

class Nit;
class Ts;
template<typename Nit> class NitWrapper;

class Controller
{
public:
    void Start() const;
    void Trigger(const NitWrapper<Nit>&);

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

private:
    Controller();
    std::fstream file;
    std::shared_ptr<Ts> ts;
    std::list<std::shared_ptr<NitWrapper<Nit>>> nitWrappers; 
};

#endif