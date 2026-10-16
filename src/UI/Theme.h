#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
class WeeTheme : public juce::LookAndFeel_V4,private juce::Timer {
 struct Hover{juce::Component::SafePointer<juce::Button> button;float amount=0;bool target=false;};
 std::vector<Hover> hover;
 void timerCallback()override{for(auto it=hover.begin();it!=hover.end();){if(!it->button){it=hover.erase(it);continue;}float target=it->target?1.f:0.f;float before=it->amount;it->amount+=.28f*(target-it->amount);if(std::abs(target-it->amount)<.01f)it->amount=target;if(before!=it->amount)it->button->repaint();++it;}}
public:
 WeeTheme(){startTimerHz(30);}
 ~WeeTheme()override{stopTimer();}
 void drawButtonBackground(juce::Graphics& g,juce::Button& button,const juce::Colour& colour,bool over,bool down)override{
  auto it=std::find_if(hover.begin(),hover.end(),[&](const auto& entry){return entry.button.getComponent()==&button;});
  if(it==hover.end()){hover.push_back({juce::Component::SafePointer<juce::Button>(&button),0,over});it=std::prev(hover.end());}it->target=over;
  auto bounds=button.getLocalBounds().toFloat().reduced(2);juce::Path outline;outline.addRoundedRectangle(bounds,9);
  juce::DropShadow(juce::Colours::black.withAlpha(.08f),4,{0,2}).drawForPath(g,outline);
  auto base=colour.interpolatedWith(juce::Colour(0xff68d4f4),it->amount*.3f);if(down)base=base.darker(.12f);if(!button.isEnabled())base=base.withMultipliedAlpha(.4f);
  g.setGradientFill(juce::ColourGradient(base.brighter(.22f),bounds.getTopLeft(),base,bounds.getBottomLeft(),false));g.fillPath(outline);g.setColour(juce::Colours::white.withAlpha(.85f));g.strokePath(outline,juce::PathStrokeType(1));
 }
};
