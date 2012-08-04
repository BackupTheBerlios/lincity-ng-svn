/*
Copyright (C) 2005 Wolfgang Becker <uafr@gmx.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <config.h>

#include "Sound.hpp"

#include <assert.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <math.h>

#include "gui/XmlReader.hpp"
#include "PhysfsStream/PhysfsSDL.hpp"

#include <SDL_mixer.h>
#include <physfs.h>
#include "Config.hpp"
#include "lincity/engglobs.cpp"

Sound* soundPtr = 0;

Sound *getSound()
{
  return soundPtr;
}

int
Sound::soundThread(void* ptr)
{
    Sound* sound = (Sound*) ptr;
    sound->loadWaves();
    return 0;
}
        
void
Sound::loadWaves() {
    //Load Waves
    std::string filename;
    std::string directory = "sounds/";
    std::string fullname;
    Mix_Chunk *chunk;
    SDL_RWops* file;
    char **rc = PHYSFS_enumerateFiles( directory.c_str() );
    char **i;
    for (i = rc; *i != NULL; i++) {
        fullname = directory;
        fullname.append( *i );
        filename.assign( *i );

        if(PHYSFS_isDirectory(fullname.c_str()))
            continue;
            
        try {        
            file = getPhysfsSDLRWops( fullname.c_str() );
            chunk = Mix_LoadWAV_RW( file, 1);
            if(!chunk) {
                std::stringstream msg;
                msg << "Couldn't read soundfile '" << fullname
                    << "': " << SDL_GetError();
                throw std::runtime_error(msg.str());
            }

            std::string idName = getIdName( filename );
            waves.insert( std::pair<std::string,Mix_Chunk*>(idName, chunk) );
        } catch(std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    PHYSFS_freeList(rc);
}
    
 /*
 * Load music theme from subfolder of 'music/'.
 * If no theme files are present, load just plain playlist.
 */
void Sound::loadMusicTheme() {
     
    //TODO there should be a music directory in
    // LINCITY_HOME
    std::string musicDir = "music/";
    //Reset track counter:
    totalTracks=0;
    playlist.clear();
    //Get the current music theme
    std::string theme = getConfig()->musicTheme;

    //Separate folder, filename and extension:
    std::string format = theme.substr((theme.length()-4), 4);
    std::string folder = "";
    std::string file = "";
    if(format == ".xml") {
        size_t found;
        found = theme.find_last_of("/");
        folder = theme.substr(0, found);
        file = theme.substr(found+1);
    }
    
    
    theme = musicDir + theme;
    
    //Check if XML file is present
    if(PHYSFS_exists(theme.c_str()) && !PHYSFS_isDirectory(theme.c_str())) {
    
        //XML file found, so let's parse it and use as a basis
        //for our music theme
        std::cerr << "File found: '" << theme << "'. Loading song data...\n";
        
        //TODO it should be possible to set lowest_tech_level and
        //highest_tech_level with tags. Something like this:
        //    <group lowest-tech-level="0" highest-tech-level="20"
        //       <song />
        //       <song />
        //    </group>
        
        //Get the number of songs
        XmlReader reader( theme );

        while( reader.read() ) {
            if( reader.getNodeType() == XML_READER_TYPE_ELEMENT) 
            {
                const std::string& element = (const char*) reader.getName();

                if( element == "song" ) {
                    XmlReader::AttributeIterator iter(reader);
                    std::string title;
                    std::string filename;
                    float lowest_tech_level = 0.0;
                    float highest_tech_level = 1000.0;
                    
                    while( iter.next() )
                    {
                        const char* name = (const char*) iter.getName();
                        const char* value = (const char*) iter.getValue();

                        if (strcmp(name, "title" ) == 0)
                            title = value;
                        else if (strcmp(name, "filename" ) == 0) {
                            filename = musicDir + folder + "/" + value;
                        }
                        else if (strcmp(name, "highest-tech-level" ) == 0)
                            highest_tech_level = strtod(value, NULL);
                        else if (strcmp(name, "lowest-tech-level" ) == 0)
                            lowest_tech_level = strtod(value, NULL);
                    }
                    song tempSong;
                    tempSong.title = title;
                    tempSong.filename = filename;
                    tempSong.trackNumber = totalTracks;
                    tempSong.lowestTechLevel = lowest_tech_level;
                    if(highest_tech_level == 0)
                        highest_tech_level = 1000.0;
                    tempSong.highestTechLevel = highest_tech_level;
                    
                    playlist.push_back(tempSong);
                    std::cerr << "Found song: '" << playlist[totalTracks].title << "'\n";
                    totalTracks++;
                }
                else {
                    std::cerr << "Config::load# Unknown element '" << element << "' in "<< theme << ".\n";
                }
            }
        }
    }
    else {
        std::string directory;
        
        if (PHYSFS_isDirectory(theme.c_str()))
            directory = theme;
        else if(PHYSFS_isDirectory(folder.c_str()))
            directory = folder;
        else
            return;
    
        //No XML file found, let's just load plain song files to the playlist:
        
        //TODO not tested. It should work, but who knows?
        
        std::string filename;
        std::string fullname;
        std::cerr << "Loading song data from '" << directory << "'...\n";

        
        //list files in 'music/'
        char **files= PHYSFS_enumerateFiles(directory.c_str());
        char **fptr=files;

        //reset the pointer
        fullname = "";
        
        //Fill the playlist with data:
        while(*fptr)
        {
            fullname = directory;
            fullname.append( *fptr );
            filename.assign( *fptr );

            if(!PHYSFS_isDirectory(fullname.c_str()) && filename[0]!='.'){
                song tempSong;
                tempSong.title = *fptr;
                tempSong.filename = directory + "/" + *fptr;
                tempSong.trackNumber = totalTracks;
                tempSong.lowestTechLevel = -10;
                tempSong.highestTechLevel =  MAX_TECH_LEVEL;
                playlist.push_back(tempSong);
                std::cerr << "Found song: '" << playlist[totalTracks].title << "'\n";
                totalTracks++;
            }
            fptr++;
            

        }
        
        PHYSFS_freeList(files);

    }
}

Sound::Sound()
    : currentMusic(0)
{
    assert( soundPtr == 0);
    soundPtr = this;
    loaderThread = 0;

    //Load Sound
    audioOpen = false;
    /* Open the audio device */
    if (Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return;
    } else {
        audioOpen = true;
        loaderThread = SDL_CreateThread(soundThread, this);
    }

    setMusicVolume(getConfig()->musicVolume);
    setSoundVolume(getConfig()->soundVolume);
    
    totalTracks = 0;
    loadMusicTheme();
    
    //'totalTracks' gets too high value in while loop. Let's fix it.
    totalTracks = totalTracks-1;
    
    //Load background music.
    //First check if there really is something in playlist to prevent crashing
    if(!playlist.empty()) {
        currentTrack = playlist[0];
    }
    playMusic();
}

Sound::~Sound()
{
    //SDL_KillThread( loaderThread );
    SDL_WaitThread( loaderThread, NULL );
    if(currentMusic)
        Mix_FreeMusic(currentMusic);

    if( soundPtr == this )
    {
        soundPtr = 0;
    }
    for (chunks_t::iterator i = waves.begin(); i != waves.end(); i++) {
        Mix_FreeChunk( i->second );
    }
    if ( audioOpen ) {
        Mix_CloseAudio();
        audioOpen = false;
    }
}

/*
 *  Playback an Audio-Effect.
 *  Name is the Name of an Audiofile from sounds/ minus .wav
 *  and without trailing Numbers. If there are eg.
 *  beep1.wav, beep2.wav, beep3.wav
 *  playSound( "beep" ) would pick one of the three Files randomly
 */
void
Sound::playSound(const std::string& name) {
    if( !getConfig()->soundEnabled ){
        return;
    }
    if( !audioOpen ){
        return;
    }

    chunks_t::size_type count = waves.count( name );
    if ( count == 0 ) {
        std::cout << "Couldn't find audio file '" << name << "'\n";
        return;
    }

    chunks_t::iterator it = waves.find(name);
    for (int i = rand() % count; i > 0; i--) {
        it++;
    }
    
    Mix_Volume( 0, getConfig()->soundVolume );
    Mix_PlayChannel( 0, it->second, 0 ); 
}

/*
 * Get ID-String for a given Filename.
 */
std::string
Sound::getIdName(const std::string& filename)
{
    std::string::size_type pos = filename.find_first_of(".0123456789");

    return filename.substr(0, pos);
}

/*
 * Change backround music.
 * Possible variables can be found from Sound.hpp: enum musicTransport
 * Currently there are three of them: NEXT_TRACK, NEXT_OR_FIRST_TRACK, PREV_TRACK
 * Sould be self-explanatory.
 */
void
Sound::changeTrack(MusicTransport command)
{
    
    //Something may gone wrong in the initialization:
    if(playlist.empty())
        return;
    
    switch(command) {
        case NEXT_TRACK:
            if (currentTrack.trackNumber+1 <= totalTracks) {
                currentTrack=playlist[currentTrack.trackNumber+1];
                playMusic();
            }
            break;
            
        case NEXT_OR_FIRST_TRACK:
            if (currentTrack.trackNumber+1 <= totalTracks) {
                currentTrack=playlist[currentTrack.trackNumber+1];
                playMusic();
            } else {
                //Jump to the beginning
                currentTrack=playlist[0];
                playMusic();
            }
            break;
            
        case PREV_TRACK:
            if (currentTrack.trackNumber > 0) { 
                currentTrack=playlist[currentTrack.trackNumber-1];
                playMusic();
            } else {
                //Jump to the beginning
                currentTrack=playlist[0];
                playMusic();
            }
            break;
    }

}


void
Sound::playMusic()
{
    if(!audioOpen)
        return;

    if(getConfig()->musicEnabled) {
        if(currentMusic) {
            Mix_FreeMusic(currentMusic);
            currentMusic = 0;
        }
        
        // I don't know if the following has any meaning at all

        //if(currentTrack.filename == "")
        //   return;
        
        //Check if current track is allowed at this tech level
        //This calculates the right tech_level and rounds it by one decimal.
        float current_tech = tech_level * (float)100 / MAX_TECH_LEVEL;
        current_tech = round(current_tech*10)/10;
        
        if(current_tech < currentTrack.lowestTechLevel
            || current_tech > currentTrack.highestTechLevel) {
            std::cerr << "Next track is " << currentTrack.title
            << " and it's tech level prerequisites range from "
            << currentTrack.lowestTechLevel << " to " << currentTrack.highestTechLevel << ".\n";
            std::cerr << "Current tech level is " << current_tech << ".\n";
            changeTrack(NEXT_OR_FIRST_TRACK);
            return;
        }

        // transform filename... because the music commands in SDL_Mixer don't
        // support reading callbacks to read from physfs directly
        
        const char* dir = PHYSFS_getRealDir(currentTrack.filename.c_str());
        if(dir == 0) {
            std::cerr << "Warning couldn't find music file '" << currentTrack.filename << "'.\n";
            return;
        }
        std::string filename = dir;
        filename += "/";
        filename += currentTrack.filename;

        
        currentMusic = Mix_LoadMUS(filename.c_str());
        if(currentMusic == 0) {
            std::cerr << "Couldn't load music file '" << filename << "': "
                << SDL_GetError() << "\n";
            return;
        }

        Mix_PlayMusic(currentMusic, 1);
    }
}

void
Sound::enableMusic(bool enabled)
{
    if(getConfig()->musicEnabled == enabled)
        return;
    getConfig()->musicEnabled = enabled;

    if(!audioOpen)
        return;

    if(enabled) {
        playMusic();
    } else {
        if(Mix_PlayingMusic()) {
            Mix_FadeOutMusic(1000);
        }
    }
}

void
Sound::setMusicVolume(int vol)
{
    if(vol < 0 || vol > 100)
        throw std::runtime_error("Music volume out of range (0..100)");

    getConfig()->musicVolume = vol;
    float volvalue = vol * MIX_MAX_VOLUME / 100.0;
    Mix_VolumeMusic(static_cast<int>(volvalue));
}

void
Sound::setSoundVolume(int vol)
{
    if(vol < 0 || vol > 100)
        throw std::runtime_error("Music volume out of range (0..100)");

    getConfig()->soundVolume = vol;
    float volvalue = vol * MIX_MAX_VOLUME / 100.0;
    Mix_Volume(-1, static_cast<int>(volvalue));
}


/** @file lincity-ng/Sound.cpp */

