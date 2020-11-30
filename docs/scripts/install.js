let clr = 0, inc = 1;

function setup() {
  createCanvas(windowWidth - 17, 500);
}

function draw() {
	noStroke();
	for (let a = 0; a < height / 5; ++a) {
		for (let b = 0; b < width / 5; ++b) {
			fill((150 + clr) - dist(b, a, 0, 0));
			rect(b * 5, a * 5, 5, 5);
		}
	}

	if (clr >= 60 || clr <= -40)
		inc *= -1;

    clr += inc;
    
    background(0);
}