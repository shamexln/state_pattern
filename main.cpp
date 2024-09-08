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
    serial::Serial m_serial{ "COM6", 19200, serial::Timeout::simpleTimeout(1000) };

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
// Vendor Code
class TransmitDeviceComponentInformation_VendorCode_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
};

// Transmit Device Component Information
// Serial Number
class TransmitDeviceComponentInformation_SerialNumber_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
};

// Transmit Device Component Information
// Hardware Revision
class TransmitDeviceComponentInformation_HardwareRevision_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
};

// Transmit Device Component Information
// Software Revision
class TransmitDeviceComponentInformation_SoftwareRevision_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
};

// Transmit Device Component Information
// Product Name
class TransmitDeviceComponentInformation_ProductName_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
};

// Transmit Device Component Information
// Part Number
class TransmitDeviceComponentInformation_PartNumber_State : public State {
public:

    void HandleData() override;
    std::vector<uint8_t> GetCommand() override;
    size_t GetRespondBytes();
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

    std::vector<uint8_t> data{ 0x10, 0x02, 0x02, 0xff, 0xed };
    return data;
}

size_t GetIntervalBaseTimeState::GetRespondBytes() {
    return GetCommand().size();
}


void TransmitDeviceComponentInformation_VendorCode_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read == 23 && rddata[0]== 0x5b && rddata[1] == 0x06 && rddata[2] == 0x0a && rddata[3] == 0x14)
        {
            // sucess
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_SerialNumber_State);
        }
        else 
        {
            // fail
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_VendorCode_State);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_VendorCode_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc };
    return data;
}

size_t TransmitDeviceComponentInformation_VendorCode_State::GetRespondBytes() {
    return 12;
}

void TransmitDeviceComponentInformation_SerialNumber_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06 && rddata[1] == 0x0a && rddata[2] == 0x14)
        {
            // sucess
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_HardwareRevision_State);
        }
        else if (bytes_read != 0 && rddata[0] == 0x15 && rddata[1] == 0x0a && rddata[2] == 0x01)
        {
            // fail
            this->context_->TransitionTo(new StopContinuousDataState);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_SerialNumber_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x26 };
    return data;
}

size_t TransmitDeviceComponentInformation_SerialNumber_State::GetRespondBytes() {
    return 12;
}

void TransmitDeviceComponentInformation_HardwareRevision_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06 && rddata[1] == 0x0a && rddata[2] == 0x14)
        {
            // sucess
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_SoftwareRevision_State);
        }
        else if (bytes_read != 0 && rddata[0] == 0x15 && rddata[1] == 0x0a && rddata[2] == 0x01)
        {
            // fail
            this->context_->TransitionTo(new StopContinuousDataState);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_HardwareRevision_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x27 };
    return data;
}

size_t TransmitDeviceComponentInformation_HardwareRevision_State::GetRespondBytes() {
    return 12;
}

void TransmitDeviceComponentInformation_SoftwareRevision_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06 && rddata[1] == 0x0a && rddata[2] == 0x14)
        {
            // sucess
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_ProductName_State);
        }
        else if (bytes_read != 0 && rddata[0] == 0x15 && rddata[1] == 0x0a && rddata[2] == 0x01)
        {
            // fail
            this->context_->TransitionTo(new StopContinuousDataState);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_SoftwareRevision_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x28 };
    return data;
}

size_t TransmitDeviceComponentInformation_SoftwareRevision_State::GetRespondBytes() {
    return 12;
}

void TransmitDeviceComponentInformation_ProductName_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06 && rddata[1] == 0x0a && rddata[2] == 0x14)
        {
            // sucess
            this->context_->TransitionTo(new TransmitDeviceComponentInformation_PartNumber_State);
        }
        else if (bytes_read != 0 && rddata[0] == 0x15 && rddata[1] == 0x0a && rddata[2] == 0x01)
        {
            // fail
            this->context_->TransitionTo(new StopContinuousDataState);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_ProductName_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x29 };
    return data;
}

size_t TransmitDeviceComponentInformation_ProductName_State::GetRespondBytes() {
    return 12;
}


void TransmitDeviceComponentInformation_PartNumber_State::HandleData() {
    {
        std::vector<uint8_t> rddata;
        std::cout << "ConcreteStateB handles request2.\n";
        std::cout << "ConcreteStateB wants to change the state of the context.\n";
        size_t bytes_wrote = this->context_->SendCmd(this);
        size_t bytes_read = this->context_->ReadRespond(this, rddata);
        if (bytes_read != 0 && rddata[0] == 0x06 && rddata[1] == 0x0a && rddata[2] == 0x14)
        {
            // sucess
            //this->context_->TransitionTo(new TransmitDeviceComponentInformation_PartNumber_State);
        }
        else if (bytes_read != 0 && rddata[0] == 0x15 && rddata[1] == 0x0a && rddata[2] == 0x01)
        {
            // fail
            this->context_->TransitionTo(new StopContinuousDataState);
        }
    }
}


std::vector<uint8_t> TransmitDeviceComponentInformation_PartNumber_State::GetCommand() {

    std::vector<uint8_t> data{ 0x10, 0x0a, 0x0a, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x30 };
    return data;
}

size_t TransmitDeviceComponentInformation_PartNumber_State::GetRespondBytes() {
    return 12;
}
/**
 * The client code.
 */
void ClientCode() {
    Context* context = new Context(new StopContinuousDataState);
    while (true)
    {
        context->Request1();
    }
    

    delete context;
}

int main() {
    ClientCode();
    return 0;
}
