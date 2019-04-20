# ActivityFace Pebble Watchface

Not too long ago there was a tech company called [Pebble](https://en.wikipedia.org/wiki/Pebble_(watch)) who made fabulous watches. What differentiated them from the competition at the time was a sterling 7+ day bettery life and amazingly clear always-on low power draw eink screens. I own several Pebble watches from the OG monochome to the Pebble Time and Pebble 2. About the only device I didn't own was the Pebble Round (although I did develop for it using the simulator).

Aside from wearing my Pebble watch(es) day in, day out, I also wrote a number of games and watchfaces utilising the C based API. Activity Face is one such watchface that I was particuarly happy with and received a positive reaction from the community. Consisting of two sets of bars for each hour (midnight to 11am on the top and 12pm to 11pm on the bottom) the watchface shows the intensity of exercise within each hour of the last day verses an average over the last 7 days for each hour.

![Watchface images](/res/promo.png)

I discontinued work on the watchface after Pebble folded, although the app is still available on the community maintained [Rebble app store](https://apps.rebble.io/en_US/application/570e63458bf1c34731000004).

I decided belated to open-source the watchface in the hope someone might benefit from understanding how it was coded (or might want to extend it.. etc). This repo contains the complete source for the latest released version (1.9) of ActivityFace supporting all Pebble watch hardware.
