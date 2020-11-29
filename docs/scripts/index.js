let clr = 0, inc = 1;

let stars = []

class Star {
	constructor() {
		this.x = random(0, width);
		this.y = random(0, height);
		this.brightness = random(0, 255);
		this.inc = random(3, 5);
	}

	display() {
		fill(255, 255, 255, this.brightness);
		noStroke();
	  circle(this.x, this.y, 1);

		this.brightness += this.inc;

		if (this.brightness >= 255) {
			this.inc *= -1;
		}
		else if (this.brightness <= 0) {
			this.inc *= -1;
			this.x = random(0, width);
			this.y = random(0, height);
		}
	}
}

function setup() {
  createCanvas(windowWidth - 17, 500);

	for (let a = 0; a < 50; ++a) {
		stars.push(new Star());
	}
}

function draw() {
	// background

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

	// stars

	for (let a = 0; a < stars.length; ++a) {
		stars[a].display();

		if (a % 2 == 0 && dist(stars[a].x, stars[a].y, mouseX, mouseY) < 200) {
			stroke(255);
			strokeWeight(2);
			line(stars[a].x, stars[a].y, mouseX, mouseY);
		}
	}
}