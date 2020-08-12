// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "midilib.hh"
#include <bse/signalmath.hh>
#include "bse/internal.hh"

#define MDEBUG(...)     Bse::debug ("midilib", __VA_ARGS__)

namespace Bse {

/// Namespace used for Midi Processor implementations.
namespace MidiLib {
using namespace AudioSignal;

// == MidiInputImpl ==
class MidiInputImpl : public MidiInputIface {
  void
  query_info (ProcessorInfo &info) override
  {
    info.uri = "Bse.MidiLib.MidiInput";
    info.version = "0";
    info.label = "MIDI-Input";
    info.category = "Input & Output";
  }
  void
  initialize () override
  {}
  void
  configure (uint n_ibusses, const SpeakerArrangement *ibusses, uint n_obusses, const SpeakerArrangement *obusses) override
  {
    remove_all_buses();
    prepare_event_input();
    prepare_event_output();
  }
  void
  reset() override
  {}
  void
  render (uint n_frames) override
  {
    EventStream &evout = get_event_output(); // needs prepare_event_output()
    EventRange evinp = get_event_input();
    for (const auto ev : evinp)
      evout.append (ev.frame, ev);
  }
};
static auto midilib_midiinputimpl = Bse::enroll_asp<MidiInputImpl>();

} // MidiLib
} // Bse
