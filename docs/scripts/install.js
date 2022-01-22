let cells = [];

class Point
{
	constructor(x, y)
	{
		this.x = x;
		this.y = y;
	}
}

function setup()
{
	createCanvas(windowWidth - 17, 500);
}

function draw()
{
	for (let i = 0; i < 50; i += 3.5)
	{
		for (let j = 0; j < 50; j += 2)
		{
			let x = j * (width / 50) + 17;
			let y = i * (height / 50) + 17;

			strokeWeight(10);
			stroke(dist(mouseX, mouseY, x, y));
			point(x, y);
		}
	}
}