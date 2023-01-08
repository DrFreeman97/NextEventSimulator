#pragma once
#include <fstream>
#include <iostream>

class IDataProvider
{
  protected:
    virtual void Next() = 0;

  public:
    virtual double GetArrival() = 0;
    virtual double GetService() = 0;
    virtual double GetInterArrival() = 0;
    virtual bool end() = 0;
};

class BaseDataProvider : public IDataProvider
{
  protected:
    double _data[3] = {0.0, 0.0, 0.0};
    bool _dataUsed[3] = {false, false, false};

  public:
    // Ereditato tramite IDataProvider
    virtual double GetArrival() override;
    virtual double GetService() override;
    virtual double GetInterArrival() override;
    BaseDataProvider();
};

class TraceDrivenDataProvider : public BaseDataProvider
{
  private:
    std::ifstream &_filestream;
    bool _end = false;
    bool _init = false;
    bool _interArrivalMode = false;
    bool IsEof() const
    {
        return _filestream.eof() || _end;
    }

  protected:
    void Next() override;
    public:
    TraceDrivenDataProvider(std::string &filePath, bool interArrivalMode = false);
    TraceDrivenDataProvider();
    virtual bool end() override
    {
        return IsEof();
    }
};

class DoubleStreamNegExpRandomDataProvider : public BaseDataProvider
{
  private:
    double _endTime = 0;
    bool _init=false;
  public:
    bool end() override;
    void Next() override;
    DoubleStreamNegExpRandomDataProvider(double endTime);
};
