# TODO

### Particles:
- [ ] Make water disperse faster
- [ ] Add gravity acceleration
- [ ] Add fire
- [ ] Add smoke
- [ ] When emitter's spawning register is empty - look for nearby particle to set it's register

### Actual particles:
- [ ] Add particles
  - [ ] Particles when landing

### Brush:
- [ ] Add circle drawing mode for brush
- [x] Rework how brush works when drawing in "lines"
  - [ ] Remove excessive overdrawing

### Biggies:
- [ ] Multiple canvases
  - [ ] Remove bedrock border and add proper boundary checks
- [ ] Multithread or compute shader

### UI/UX:
- [ ] Zoom and move around
- [ ] UI for selecting interaction mode
- [x] Particle tooltip on mouse hover

### Sounds:
- [ ] Sounds
  - [ ] When particle lands?

### Physics
- [ ] PHYSICS

### Debug and profile:
- [ ] Display graph or output data of each frame from profiler
- [ ] Add ability to select and modify speific particle in debug mode
- [ ] When stepping through simulation - select with the mouse where next simulation of a particle whould occur
- [x] Add frame limit to profiler and collect data for each frame
- [x] Stop simulation
  - [x] Step particle by particle through simulation
  - [x] Step through whole simulation phase

### Other:
- [ ] Add config file
  - [ ] Config brush size increment