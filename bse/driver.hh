// This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0
#ifndef __BSE_DRIVER_HH__
#define __BSE_DRIVER_HH__

#include <bse/midievent.hh>
#include <functional>

namespace Bse {

class Driver : public std::enable_shared_from_this<Driver> {
protected:
  struct Flags { enum { OPENED = 1, READABLE = 2, WRITABLE = 4, }; };
  const String       devid_;
  size_t             flags_ = 0;
  explicit           Driver     (const String &devid);
  virtual           ~Driver     ();
  template<class Derived> std::shared_ptr<Derived>
  /**/               shared_from_base () { return std::static_pointer_cast<Derived> (shared_from_this()); }
public:
  enum {
    JACK      = 0x01 << 24,
    ALSA_THRU = 0x10 << 24,
    ALSA_USB  = 0x16 << 24,
    ALSA_KERN = 0x17 << 24,
    OSS       = 0x20 << 24,
    PULSE     = 0x40 << 24,
    ALSA_USER = 0x50 << 24,
    PSEUDO    = 0x70 << 24,
    PAUTO     = 0x74 << 24,
    PNULL     = 0x77 << 24,
    WCARD     = 0x01 << 16,
    WDEV      = 0x01 <<  8,
    WSUB      = 0x01 <<  0,
  };
  enum IODir { READONLY = 1, WRITEONLY = 2, READWRITE = 3 };
  typedef std::shared_ptr<Driver> DriverP;
  bool           opened        () const        { return flags_ & Flags::OPENED; }
  bool           readable      () const        { return flags_ & Flags::READABLE; }
  bool           writable      () const        { return flags_ & Flags::WRITABLE; }
  virtual String devid         () const        { return devid_; }
  virtual void   close         () = 0;
  // registry
  using Entry = DriverEntry;
  using EntryVec = DriverEntrySeq;
};
using DriverP = Driver::DriverP;

class MidiDriver : public Driver {
protected:
  explicit           MidiDriver      (const String &devid);
  virtual Bse::Error open            (IODir iodir) = 0;
public:
  typedef std::shared_ptr<MidiDriver> MidiDriverP;
  static MidiDriverP open            (const String &devid, IODir iodir, Bse::Error *ep);
  virtual bool       has_events      () = 0;
  virtual uint       fetch_events    (AudioSignal::EventStream &estream, double samplerate) = 0;
  static EntryVec    list_drivers    ();
  static String      register_driver (const String &driverid,
                                      const std::function<MidiDriverP (const String&)> &create,
                                      const std::function<void (EntryVec&)> &list);
};
using MidiDriverP = MidiDriver::MidiDriverP;

struct PcmDriverConfig {
  uint n_channels = 0;
  uint mix_freq = 0;
  uint latency_ms = 0;
  uint block_length = 0;
};

class PcmDriver : public Driver {
protected:
  explicit           PcmDriver       (const String &devid);
  virtual Bse::Error open            (IODir iodir, const PcmDriverConfig &config) = 0;
public:
  typedef std::shared_ptr<PcmDriver> PcmDriverP;
  static PcmDriverP  open            (const String &devid, IODir desired, IODir required, const PcmDriverConfig &config, Bse::Error *ep);
  virtual bool       pcm_check_io    (long *timeoutp) = 0;
  virtual void       pcm_latency     (uint *rlatency, uint *wlatency) const = 0;
  virtual float      pcm_frequency   () const = 0;
  virtual uint       block_length    () const = 0;
  virtual size_t     pcm_read        (size_t n, float *values) = 0;
  virtual void       pcm_write       (size_t n, const float *values) = 0;
  static EntryVec    list_drivers    ();
  static String      register_driver (const String &driverid,
                                      const std::function<PcmDriverP (const String&)> &create,
                                      const std::function<void (EntryVec&)> &list);
};
using PcmDriverP = PcmDriver::PcmDriverP;

} // Bse

#endif  // __BSE_DRIVER_HH__
