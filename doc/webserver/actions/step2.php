<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
 <!--

  File :  $Source$

  Purpose  :  Submit form for actions file feedback (step 2)
              This file belongs in
              ijbswa.sourceforge.net:/home/groups/i/ij/ijbswa/htdocs/

  $Id$

  $Log$
  Revision 1.10  2002/04/06 15:19:35  oes
  Clean-up, smarter handling of unreachable URLs

  Revision 1.6  2002/04/02 07:22:19  oes
  Elimnating duplicate images; using relative link for step3

  Revision 1.4  2002/04/01 19:13:47  oes
  Extended, fixed bugs, beefed up design, made IE-safe

  Revision 1.1  2002/03/30 03:20:30  oes
  Added Feedback mechanism for actions file


  Copyright (C) 2002 the SourceForge Privoxy team.
  http://www.privoxy.org/

  Written by Andreas Oesterhelt

  This program is free software; you can redistribute it
  and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at
  your option) any later version.

  This program is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  The GNU General Public License should be included with
  this file.  If not, you can view it at
  http://www.gnu.org/copyleft/gpl.html
  or write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 -->

 <head>
  <meta http-equiv="Content-Style-Type" content="text/css">
  <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
  <link rel="stylesheet" type="text/css" href="../p_feedback.css">

  <script language="javascript" type="text/javascript">
  <!--
   //
   // Could be as easy as style="max-wdith: 300px; max-height..." inside the
   // <img> tag, but IE doesn't do that. Setting the values directly also
   // screws IE for some weird reason. All praise MS.
   //

   function prettyscale(image)
   {
      newwidth = 0
      newheight = 0

      if (image.width > 300)
      {
         newwidth = 300
      }

      if (image.height > 50)
      {
         newheight = 50
      }

      if (image.width < 20)
      {
         newwidth = 20
      }

      if (image.height < 20)
      {
         newheight = 20
      }

      if (newwidth != 0)
      {
         image.width = newwidth
      }

      if (newheight != 0)
      {
         image.height = newheight
      }
   }
  //-->
  </script>


<?php

/*
 * For testing: 
 */
//phpinfo();
error_reporting(E_ALL);
//error_reporting(E_NONE);

/*
 * Function: link_to_absolute
 * Purpose:  Make links from $base absolute
 */
function link_to_absolute($base, $link)
{
   /*
    * If $link already is absolute, we're done:
    */
   if (!strncmp("http://", $link, 7) || !strncmp("https://", $link, 8))
   {
      return $link;
   }

   /*
    * Cut the base to it's proto://host/ or to its proto://host/dir/,
    * depending whether $link is host-relative or path-relative.
    */
   if ($link{0} == "/")
   {
      /*
       * host-relative:
       */
       preg_match('|^(https?://[^/]+)|i', $base, $results);
       $base = $results[1];
   }
   else
   {
      /*
       * path-relative:
       */
      if (strpos($base, '/') != strlen($base))
      {
         $base = substr($base, 0, -strpos(strrev($base), '/'));
      }
   }
   return $base.$link;
}

/*
 * Function: error_abort
 * Purpose:  Return an error page with $title and $message
 */
function error_abort($title, $message)
{
   if ($title == "invalid") /* shortcut */
   {
      $title = "Invalid Feedback Submission";
   }

   echo ("  <title>Privoxy: $title</title>
           </head>
           <body>
            <div class=\"title\">
             <h1>
              <a href=\"http://www.privoxy.org/\">Privoxy</a>: $title
              </h1>
             </div>
            <center>
             <div class=\"errorbox\">
              $message
             </div>
            </center>
            <p>Valid <a href=\"http://validator.w3.org/\">HTML 4.01 Transitional</a></p>
           </body>
          </html>\n");
   exit; 
}

/* 
 * Cannot start with step 2:
 */
if (!isset($referrer_url))
{
   error_abort("invalid", "When submitting your feedback please start with
                <a href=\"index.php\">step 1</a>.");
}


/* 
 * Cannot work on unknown problem:
 */
if (!isset($problem) || $problem == "INVALID")
{
   error_abort("invalid", "You need to select the nature of the problem in
                <a href=\"javascript:history.back();\">step 1</a>.");
}


/*
 * If the protocol is missing from $referrer_url, prepend "http://"
 */
if (strncmp("http://", $referrer_url, 7))
{
   $referrer_url = "http://" . $referrer_url;
}


/*
 * Check if URL really exists and buffer its contents:
 *
 * FIXME: Curl is not installed on SF; Filed as Alexandria
 *        Feature Request #537014. 
 *        PHP's fopen() supports URLs, but it seems that
 *        curls options for Timeouts and HTTP error handling
 *        are not supported by fopen().
 */


/*
 * Slurp the page in:
 */
$ch = curl_init ($referrer_url);

curl_setopt ($ch, CURLOPT_HEADER, 0);
curl_setopt ($ch, CURLOPT_FAILONERROR, 1);
curl_setopt ($ch, CURLOPT_FOLLOWLOCATION, 1);
curl_setopt ($ch, CURLOPT_TIMEOUT, 20);            

ob_start();
$success = curl_exec ($ch);
$page = ob_get_contents();
ob_end_clean();

curl_close ($ch);

if (!$success)
{
   $url_confirm = "
     <dt>
      <p><b>Confirm the URL:</b></p>
     </dt>
     <dd>
      <p>
       The URL that you entered could not be retrieved. Please make sure that
      </p>
      <p class=\"important\">
       <a href=\"$referrer_url\">$referrer_url</a>
      </p>
      <p>
       is correct and publicly accssible.
      </p>
      <p>
       <input type=\"checkbox\" name=\"url_confirmed\" value=\"user\"> Yes, I'm sure.
      </p>
     </dd>";
}
else
{
   $url_confirm = "<input type=\"hidden\" name=\"url_confirmed\" value=\"automatic\">";
}

/* 
 * Create description from problem code:
 */
switch($problem)
{
   case "P1": $problem_description="an advertisment was not blocked"; break;
   case "P2": $problem_description="an innocent image was blocked"; break;
   case "P3": $problem_description="the whole page was erraneously blocked"; break;
   case "P4": $problem_description="the page needs popups but they don't work"; break;
   case "P5": $problem_description="a problem occured"; break;
   default: $problem_description="AN UNPROCESSABLE PROBLEM OCCURED";
}

?>

  <title>Privoxy Action List Feedback - Step 2 of 2</title>
 </head>
 <body>

  <div class="title">
   <h1>
     <a href="http://www.privoxy.org" target="_blank">Privoxy</a> Action List Feedback - Step 2 of 2
   </h1>
  </div>

  <div class="box">
   <b>You are about to report that <?php echo ($problem_description) ?> on
   <a href="<?php echo ($referrer_url) ?>"><?php echo ($referrer_url) ?></a>.</b>
  </div>

  <div class="box">
   <form action="step3.php" method="post">
    <p>
     <input type="hidden" name="problem" value="<?php echo ($problem) ?>">
     <input type="hidden" name="referrer_url" value="<?php echo ($referrer_url) ?>">
    </p>

    <dl>

<?php

/*
 * Include the confirmation for an unretrievable URL if
 * necessary
 */
echo ($url_confirm);

/*
 * Create / suppress form elements depending on type of
 * problem
 */
if ($problem != "P1")
{
   echo ("<!--");
}
else
{
   /*
    * Extract all image links from page, make them
    * absolute, and present them (scaled to reasonable size)
    * in a table for the user to select
    */

   preg_match_all('|<img\s+[^>]*?src=[\'"]?(.*?)[\'" >]|i', $page, $matches);
   $image_urls = array_values(array_unique($matches[1]));
   $count = count($image_urls);

   if ($count > 0)
   {
      /*
       * Open section in <dl>; Open table:
       */
      echo ("     <dt><b>Choose the images you want blocked from the following list:</b></dt>
                  <dd>
                   <p>
                    <input type=\"hidden\" name=\"num_images\" value=\"$count\">
                    <table border=\"0\" cellpadding=\"0\" cellspacing=\"4\">\n");
      /*
       * Print one table row for each image found:
       */
      for ($i=0; $i< $count; $i++)
      {
         $image_url = link_to_absolute($referrer_url, $image_urls[$i]);

         /*
          * Print the row(s):
          */
         echo ("       <tr>
                        <td rowspan=\"2\">
                         <input type=\"checkbox\" name=\"block_image[$i]\" value=\"off\">
                        </td>
                        <td>
                         <a href=\"$image_url\">$image_url</a>:
                        </td>
                        <td>
                         <input type=\"hidden\" name=\"image_url[$i]\" value=\"$image_url\">
                        </td>
                       </tr>
                       <tr>
                        <td>
                         <img onload=\"prettyscale(this);\" src=\"$image_url\" alt=\"banner or not?\">
                        </td>
                       </tr>\n");
      }
      echo ("      </table>
                  </dd>

                  <dt>
                   <b>If the banner that you saw is not listed above, enter the URL here</b>\n");
   }
   else
   {
      echo ("     <dt>
                   <b>URL of the advertisment image:</b>\n");
   }
}

?>

      <br><i>Hint: right-click the image, select "Copy image location" and paste the URL here.</i>
     </dt>
     <dd>
      <p>
       <input name="manual_image_url" type="text" size="45" maxlength="255">
      </p>
     </dd>

<?php if($problem != "P1") echo ("-->") ?>

<?php if($problem != "P2") echo ("<!--") ?>

     <dt>
      <p><b>URL of the innocent image:</b>
       <br><i>Hint: right-click the image, select "Copy image location" and paste the URL here.
       <br>This may not work if the image was blocked by size or if +image-blocker is set to redirect.</i>
      </p>
     </dt>
     <dd>
      <p>
       <input name="image_url" value="unknown" type="text" size="45" maxlength="255">
      </p>
     </dd>

<?php if($problem != "P2") echo ("-->") ?>

     <dt><b>Severity:</b></dt>
     <dd>
      <p>
       <select name="severity">
        <option value="3">drives me crazy</option>
        <option selected value="2">normal</option>
        <option value="1">cosmetic</option>
       </select>
      </p>
     </dd>

     <dt>
      <b>Remarks:</b> <i>(optional)</i>
     </dt>
     <dd>
      <p>
       <textarea wrap="hard" style="font-size: 10px" name="remarks" cols="35" rows="3">None.</textarea>
      </p>
     </dd>

     <dt>
      <b>Your Name:</b> <i>(optional, public)</i>
     </dt>
     <dd>
      <p>
       <input name="name" size="45">
      </p>
     </dd>

     <dt>&nbsp;</dt>
     <dd>
      <input type="submit" value="Submit">
     </dd>

    </dl>
   </form>
  </div>

  <p>Valid <a href="http://validator.w3.org/">HTML 4.01 Transitional</a></p>

 </body>
</html>
