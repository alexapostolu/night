class Node {
  constructor() {
    this.x = random(0, width);
    this.y = random(0, height);
    fill(255);
    circle(this.x, this.y, 2);
  }
}

let nodes = [];

function setup() {
  createCanvas(windowWidth - 17, 500);
  background(0);

  for (let a = 0; a < 100; ++a) {
    nodes.push(new Node());
  }
}

function draw() {}

function mousePressed() {
  let node1 = nodes[0],
    node2 = nodes[1],
    node3 = nodes[2];
  for (let a = 2; a < nodes.length; ++a) {
    if (
      dist(nodes[a].x, nodes[a].y, mouseX, mouseY) <
      dist(node1.x, node1.y, mouseX, mouseY)
    ) {
      node3 = node2;
      node2 = node1;
      node1 = nodes[a];
    } else if (
      dist(nodes[a].x, nodes[a].y, mouseX, mouseY) <
      dist(node2.x, node2.y, mouseX, mouseY)
    ) {
      node3 = node2;
      node2 = nodes[a];
    } else if (
      dist(nodes[a].x, nodes[a].y, mouseX, mouseY) <
      dist(node3.x, node3.y, mouseX, mouseY)
    ) {
      node3 = nodes[a];
    }
  }

  fill(255);
  triangle(node1.x, node1.y, node2.x, node2.y, node3.x, node3.y);
}

