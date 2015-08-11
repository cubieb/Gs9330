#ifndef _Controller_h_
#define _Controller_h_

class Nit;
template<typename Nit> class NitWrapper;

class Controller
{
public:
    void Start() const;
    void Trigger(const NitWrapper<Nit>&) const;

    static Controller& GetInstance()
    {
        static Controller instance;
        return instance;
    }

private:
    Controller();
    std::shared_ptr<NitWrapper<Nit>> nitWrapper; 
};

#endif