/*********************************************************
 * This implements "external" sort
 *
 * assume that the data is in one array
 *
 * This is a 2 pass algorithm
 *
 ********************************************************/

 /* 0: divide the data into smaller chunks of ES_FILE_SIZE based on total size*/

 /* 1: sort each component and write to disk */

 /* 2: read the first ES_FILE_SIZE/(num_files+1) bytes of the data and do a num_files ways merge,
  * the last component is used for a write buffer */

 /* 3. now write the file into one file (otherwise need to keep track of file num for each column
  * which is more work) */
