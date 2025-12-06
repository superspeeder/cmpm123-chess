## Implementation details

I implemented move generation just in time to replace it with the move generation in the GameState class that was
provided (which I used because it was significantly better than my own generation code). I just used the basic
evaluation function that was suggested with very little changes (had to tweak it a bit because it kept just getting rid
of pieces).

## Final Version Challenges

I didn't really run into many big challenges. I wish I could've spent more time working on making my move generation
faster or evaluation function better, but I barely had the time to do the base version of this assignment due to work
from my other classes. I am still happy with the outcome (though that might be because once I realized that something
was wrong in my evaluation function I fixed that and it stopped throwing pieces away constantly, and im really not very
good at chess). I also wish I had time to get the fancier moves implemented.

The best depth I've been able to run it at was 6, though it's rather slow. Running at 5 produced pretty good results and
was able to beat me (I suck at chess though).

## Video

![video](video.mp4)