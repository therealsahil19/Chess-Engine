export function parseInfo(line) {
  const info = {};
  const parts = line.split(' ');

  // score
  const scoreIdx = parts.indexOf('score');
  if (scoreIdx !== -1) {
    info.scoreType = parts[scoreIdx + 1]; // cp or mate
    info.scoreVal = parseInt(parts[scoreIdx + 2], 10);

    const boundToken = parts[scoreIdx + 3];
    if (boundToken === 'lowerbound') {
      info.bound = 'lower';
    } else if (boundToken === 'upperbound') {
      info.bound = 'upper';
    }
  }

  // depth
  const depthIdx = parts.indexOf('depth');
  if (depthIdx !== -1) {
    info.depth = parseInt(parts[depthIdx + 1], 10);
  }

  // pv (principal variation - best line)
  const pvIdx = parts.indexOf('pv');
  if (pvIdx !== -1) {
    info.pv = parts.slice(pvIdx + 1).join(' ');
  }

  return info;
}
