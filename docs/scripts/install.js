let points = [];
let anchors = [];
let sep = 30;

function setup()
{
	createCanvas(windowWidth - 17, 500);

	for (let i = 0; i < height / sep; ++i)
	{
		for (let j = 0; j < width / sep; ++j)
		{
			points.push(new Point(
				(j * sep) + (sep / 2),
				(i * sep) + (sep / 2)
			));
		}
	}

	for (let i = 0; i < 15; ++i)
		anchors.push(new Anchor(random(width), random(height)));

	for (let i = 0; i < points.length; ++i)
	{
		let x = 0;
		for (let j = 1; j < anchors.length; ++j)
		{
			if (dist(points[i].x, points[i].y, anchors[j].x, anchors[j].y) < 
				dist(points[i].x, points[i].y, anchors[x].x, anchors[x].y))
				x = j;
		}

		anchors[x].points.push(points[i]);
	}

	for (let i = 0; i < anchors.length; ++i)
	{
		let lines = [], d = {};
		for (let j = 0; j < anchors[i].points.length - 1; ++j)
		{
			for (let k = j + 1; k < anchors[i].points.length; ++k)
			{
				if ((anchors[i].points[j].y === anchors[i].points[k].y && 
					 anchors[i].points[j].x === anchors[i].points[k].x - sep) ||
					(anchors[i].points[j].x === anchors[i].points[k].x && 
					 anchors[i].points[j].y === anchors[i].points[k].y - sep))
				{
					let v1 = createVector(anchors[i].points[j].x, anchors[i].points[j].y);
					let v2 = createVector(anchors[i].points[k].x, anchors[i].points[k].y);
					lines.push([ v1, v2 ]);

					if (v1 in d)
						d[v1]++;
					else
						d[v1] = 1;
					
					if (v2 in d)
						d[v2]++;
					else
						d[v2] = 1;
				}
			}
		}

		for (let j = 0; j < lines.length; ++j)
		{
			if (d[lines[j][0]] < 4 || d[lines[j][1]] < 4)
				anchors[i].lines.push(lines[j]);
		}
	}
}

function draw()
{
	noFill();
	strokeWeight(1);
	for (let i = 0; i < points.length; ++i)
		points[i].draw();
	
	strokeWeight(3);
	for (let i = 0; i < anchors.length; ++i)
	anchors[i].draw();
}

class Point
{
	constructor(x, y)
	{
		this.x = x;
		this.y = y;
	}

	draw()
	{
		stroke(255 - dist(mouseX, mouseY, this.x, this.y));
		circle(this.x, this.y, 13, 13);
	}
}

class Anchor
{
	constructor(x, y)
	{
		this.x = x;
		this.y = y;
		this.points = [];
		this.lines = [];

		this.clr = random(200);
		this.inc = random(2, 4);
		if (random(1) > 0.5)
			this.inc *= -1;
	}

	draw()
	{
		stroke(this.clr);
		this.clr += this.inc;
		if (this.clr < 0 || this.clr > 200)
			this.inc *= -1;

		for (let i = 0; i < this.lines.length; ++i)
		{
			line(this.lines[i][0].x, this.lines[i][0].y,
				 this.lines[i][1].x, this.lines[i][1].y);
		}
	}
}