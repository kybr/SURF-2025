#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/io/al_File.hpp"

using namespace al;

std::unordered_map<char, std::unordered_map<const char *, char>> map = {
    {'1',
     {
         {"Bird Chirp (long).mp3.wav", 'a'},    //
         {"Bug buzzy.mp3.wav", 'k'},            //
         {"Cleaner chirp.mp3.wav", 's'},        //
         {"Creepy plane.mp3.wav", 'v'},         //
         {"Crunchy Walk.mp3.wav", 'n'},         //
         {"E plane.mp3.wav", 'z'},              //
         {"F plane.mp3.wav", 'x'},              //
         {"F# plane.mp3.wav", 'c'},             //
         {"F# wind.mp3.wav", 'l'},              //
         {"Long walk.mp3.wav", 'b'},            //
         {"Lower pitched chirp.mp3.wav", 'd'},  //
         {"Person _hold on_.mp3.wav", 'f'},     //
         {"Person _ya_.mp3.wav", 'g'},          //
         {"Rumbling wind.mp3.wav", 'j'},        //
         {"Soft wind.mp3.wav", 'h'},            //
     }},
    {'2',
     {
         {"Another wave.mp3.wav", 'w'},      //
         {"Birds (multiple).mp3.wav", 'p'},  //
         {"Birds loud.mp3.wav", 'a'},        //
         {"Buzz (Bee_).mp3.wav", 'd'},       //
         {"Cling Clank.mp3.wav", 'i'},       //
         {"Cycling wind.mp3.wav", 's'},      //
         {"F plane.mp3.wav", 'g'},           //
         {"F# Plane.mp3.wav", 'j'},          //
         {"Footsteps 1.mp3.wav", 'l'},       //
         {"Footsteps 2.mp3.wav", ';'},       //
         {"Kid upset_.mp3.wav", 't'},        //
         {"Ocean Spray.mp3.wav", 'q'},       //
         {"People (oh no).mp3.wav", 'y'},    //
         {"People (woo).mp3.wav", 't'},      //
         {"Percussive click.mp3.wav", 'o'},  //
         {"Plane E (Loud).mp3.wav", 'f'},    //
         {"Plane F (loud).mp3.wav", 'h'},    //
         {"Plane F# vib.mp3.wav", 'k'},      //
         {"Wave Dramatic.mp3.wav", 'r'},     //
         {"Wave choppy.mp3.wav", 'e'},       //
     }},
    {'3',
     {
         {"B plane.mp3.wav", 'f'},                             //
         {"Bike (Crank).mp3.wav", 'e'},                        //
         {"Bike (long).mp3.wav", 'r'},                         //
         {"Bike (quiet to loud).mp3.wav", 't'},                //
         {"Bike (sudden rush).mp3.wav", 'y'},                  //
         {"Bike 6.mp3.wav", 'q'},                              //
         {"Bike 8.mp3.wav", 'w'},                              //
         {"Bike Creaky tires.mp3.wav", 'u'},                   //
         {"Bike click.mp3.wav", 'i'},                          //
         {"Bikepath walking clip.mp3.wav", 'l'},               //
         {"C plane.mp3.wav", 'g'},                             //
         {"More birds.mp3.wav", 's'},                          //
         {"People (he lives a different life).mp3.wav", 'p'},  //
         {"Person (im indifferent).mp3.wav", 'o'},             //
         {"Plane C# w bike.mp3.wav", 'h'},                     //
         {"Plane F w bike.mp3.wav", 'k'},                      //
         {"Plane humm.mp3.wav", 'd'},                          //
         {"Rumble D plane.mp3.wav", 'j'},                      //
         {"birds.mp3.wav", 'a'},                               //
     }}};

void load(char which,
          std::unordered_map<char, gam::SamplePlayer<float> *> &player) {
  if (which < '1' || which > '3') {
    std::cerr << "Invalid key: " << which << std::endl;
    return;
  }

  auto &keybinding = map[which];
  if (keybinding.empty()) {
    std::cerr << "No samples found for key: " << which << std::endl;
    return;
  }

  player.clear();
  for (const auto &pair : keybinding) {
    auto *p = new gam::SamplePlayer<float>(pair.first);  // Load the sample
    p->finish();
    player[pair.second] = p;
    std::cout << pair.second << " -> "  //
              << pair.first << " (" << p->size() << ")" << std::endl;
  }
}

struct MyApp : App {
  std::unordered_map<char, gam::SamplePlayer<float> *> player;

  std::mutex lock;

  void onCreate() override {}

  void onAnimate(double dt) override {}

  void onDraw(Graphics &g) override {  //
    g.clear(0);
  }

  float x1 = 0;
  float y1 = 0;
  void onSound(AudioIOData &io) override {
    while (io()) {
      float f = 0.0f;
      lock.lock();
      for (auto &p : player) {
        f += p.second->operator()();
      }
      lock.unlock();

      // DCblock filter because a lot of the samples have DC offset
      y1 = f - x1 + 0.995 * y1;
      x1 = f;

      io.out(0) = y1;
      io.out(1) = y1;
    }
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() >= '1' && k.key() <= '3') {
      lock.lock();
      load(k.key(), player);
      lock.unlock();
      return false;
    }

    if (player.find(k.key()) != player.end()) {
      player[k.key()]->reset();
      std::cout << "Playing sample: " << k.key() << std::endl;
      return false;
    }

    return false;  // Allow other keys to be processed
  }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 2);
  gam::Domain::master().spu(44100.0);
  app.start();
}
