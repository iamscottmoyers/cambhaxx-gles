<?php
  /* Create a flat voxel map from an image
     Based on my knit chart maker (http://httpcolonforwardslashforwardslash.co.uk/knit)
     Just thought it would be interesting...
   */

  /* Usage:
     php img_to_voxel.pl <image_file>  <output_array> <scale>
   */

$img = $argv[1];
$output = $argv[2];
$scale = $argv[3];

/* get file attributes */
list($width, $height, $type) = getimagesize($img);

switch(image_type_to_extension($type, $dot = false))
  {
  case 'jpeg':
    $im_orig = @imagecreatefromjpeg($img);
    break;
  case 'png':
    $im_orig = @imagecreatefrompng($img);
    break;
  case 'gif':
    $im_orig = @imagecreatefromgif($img);
    break;
  default:
    exit('unsupported file format (jpeg, gif and png supported)');
    break;
  }

$im = @imagecreatetruecolor($width/$scale, $height/$scale);

if(!$im_orig)
  exit('could not open image');

/* copy the orig image to a new size */
if(!imagecopyresized($im, $im_orig, 0, 0, 0, 0, $width/$scale, $height/$scale, $width, $height))
  exit('error, try again');

echo '/* Created by img_to_voxel.php */' . "\n";
echo 'const pos_t ' . (($output != '') ? $output : 'b') . '[] = {' . "\n";

for($y = $height/$scale; $y > 0; --$y)
  {
    for($x = $width/$scale; $x > 0; --$x)
      {
	$rgb = @imagecolorat($im, $x, $y);
	$colors = @imagecolorsforindex($im, $rgb);
	echo '{' . $x . ', ' . -$y . ', 0, {';
	echo $colors['red'] . ', ' . $colors['green'] . ', ' . $colors['blue'] . ', 1';
	echo '}},' . "\n";
      }
  }

echo '};' . "\n";

?>
