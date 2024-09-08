#include <iostream>
#include <typeinfo>
#include "serial/serial.h"
/**
 * The base State class declares methods that all Concrete State should
 * implement and also provides a backreference to the Context object, associated
 * with the State. This backreference can be used by States to transition the
 * Context to another State.
 */

class Context;

class State {
    /**
     * @var Context
     */
protected:
    Context* context_;

public:
    virtual ~State() {
    }

    void set_context(Context* context) {
        this->context_ = context;
    }
    virtual std::vector<uint8_t> GetCommand() = 0;
    virtual size_t GetRespondBytes() = 0;
    virtual void HandleData() = 0;
};

/**
 * The Context defines the interface of interest to clients. It also maintains a
 * reference to an instance of a State subclass, which represents the current
 * state of the Context.
 */
class Context {
    /**
     * @var State A reference to the current state of the Context.
     */
private:

    State* state_;
    serial::Serial m_serial{ "COM4", 19200, serial::Timeout::simpleTimeout(1000) };

public:
    Context(State* state) : state_(nullptr) {
        this->TransitionTo(state);
    }
    ~Context() {
        delete state_;
    }
    size_t SendCmd(State* state)
    {
        size_t bytes_wrote = m_serial.write(this->state_->GetCommand());
        return bytes_wrote;
    }

    size_t ReadRespond(State* state, std::vector<uint8_t>& rddata)
    {
        size_t bytes_read = m_serial.read(rddata, this->state_->GetRespondBytes());
        return bytes_read;
    }
    /**
     * The Context allows changing the State object at runtime.
     */
    void TransitionTo(State* state) {
        std::cout << "Context: Transition to " << typeid(*state).name() << ".\n";
        if (this->state_ != nullptr)
            delete this->state_;
        this->state_ = state;
        this->state_->set_context(this);
    }
    /**
     * The Context delegates part of its behavior to the current State object.
     */
    void Request1() {
        this->state_->HandleData();
    }

};

/**
 * Concrete States implement various behaviors, associated with a state of the
 * Context.
 */

class StopContinuousDataState : public State {
public:
    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes() override;
};

class GetIntervalBaseTimeState : public State {
public:

    void HandleData() override;

    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes() override;
  
};


// Transmit Device Component Information
// VendorCode
class TransmitDeviceComponentInformation_VendorCode_State : public State {
public:

    void HandleData() override {
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        this->context_->TransitionTo(new StopContinuousDataState);
    }
    std::vector<uint8_t> GetCommand() override {

        std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25 };
        return data;

    }
    size_t GetRespondBytes() {
        return 23;
    }

};

void StopContinuousDataState::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "StopContinuousDataState handles StopContinuousData.\n";
        std::cout << "StopContinuousDataState wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0)
        {
            this->context_->TransitionTo(new GetIntervalBaseTimeState);
        }
        
    }
}


std::vector<uint8_t> StopContinuousDataState::GetCommand() {
        
     std::vector<uint8_t> data{ 0x10, 0x01, 0x19, 0xd6 };
     return data;
}

size_t StopContinuousDataState::GetRespondBytes() {
    return GetCommand().size();
}

void GetIntervalBaseTimeState::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06)
        {
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_VendorCode_State);
        }

    }
}


std::vector<uint8_t> GetIntervalBaseTimeState::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x02, 0x02, 0xff, 0x14 };
    return data;
}

size_t GetIntervalBaseTimeState::GetRespondBytes() {
    return GetCommand().size();
}

/**
 * The client code.
 */
void ClientCode() {
    Context* context = new Context(new StopContinuousDataState);
    context->Request1();

    delete context;
}

int main() {
    ClientCode();
    return 0;
}
