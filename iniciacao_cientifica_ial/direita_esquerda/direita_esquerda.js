'use strict';
function direitaEsquerda(frame) {
  const left_color = red;
  const right_color = blue;
  const line_color = green;
  const vector_color = purple;

  const vector = new Container();
  const vector_line = new Line({
    color: vector_color
  }).drag().addTo(vector);
  const vector_start = new Circle(10, vector_color).drag().addTo(vector);
  const vector_end = new Triangle(20, 30, 30, vector_color).drag().addTo(vector).mov(0,-50);
  vector.center();
  vector.on("pressup", on_pressmove);
  vector.on("pressmove", on_pressmove);


  const points = new Array();

  let line_dirty = true;
  let line_normal = {};
  let line_k = 0;

  update_line();
  redraw();
  frame.stage.on("stagemouseup", on_click);

  function update_line () {
    const M = vector.getMatrix();
    const parent_start = M.transformPoint(vector_start.x, vector_start.y);
    const parent_end = M.transformPoint(vector_end.x, vector_end.y);
    const tangent = difference(vector_start, vector_end);
    const parent_tangent = difference(parent_start, parent_end);
    line_normal.x = -parent_tangent.y;
    line_normal.y = parent_tangent.x;

    line_k = dot_product(line_normal, parent_start);

    if(line_dirty)
    {
      vector_end.rot(-Math.atan2(tangent.x, tangent.y) * 180 / Math.PI);
      const coord_change = vector_line.getMatrix().invert();
      vector_line
        .from(coord_change.transformPoint(vector_start.x, vector_start.y))
        .to(coord_change.transformPoint(vector_end.x, vector_end.y));
      line_dirty = false;
      zog('Normal', line_normal, ' k', line_k);
zog('Matrix', M);
    }
  }


  function redraw () {
    points.forEach((p) => update_point(p));
    frame.update();
  }

  function update_point (p) {
      const k = dot_product(line_normal, p);
      if(k === line_k) p.color = line_color;
      else if(k > line_k) p.color = right_color;
      else if(k < line_k) p.color = left_color;
  }

  function on_click(e) {
    if(null == e.relatedTarget) {
      const point = new Circle(5).pos(e.stageX, e.stageY).addTo();
      update_point(point);
      points.push(point);
      frame.update();
    }
  }

  function on_pressmove(e) {
    if(vector_line == e.target) {
      const start = [vector_line.startX, vector_line.startY];
      const end = [vector_line.endX, vector_line.endY];

      const vs = vector_line.localToLocal(...start, vector_start.parent);
      const ve = vector_line.localToLocal(...end, vector_end.parent);
      vector_start.x = vs.x;
      vector_start.y = vs.y;
      vector_end.x = ve.x;
      vector_end.y = ve.y;
    }
    else
    {
      line_dirty = true;
    }
    update_line();
    redraw();
  }
}


function dot_product (a, b) {
  return a.x * b.x + a.y * b.y;
}

function difference (a, b) {
  return {x: a.x - b.x, y: a.y - b.y};
}

