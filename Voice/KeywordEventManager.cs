using UnityEngine;
using UnityEngine.Events;
using UnityEngine.WSA.Speech;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// The keyword event manager lets you create keyword event handlers within the Unity IDE.  To create event handlers
/// add this script to any game object and expand the keywordHandlers list.
/// </summary>
public class KeywordEventManager : MonoBehaviour
{
    [System.Serializable]
    public class Keyword
    {
        [Tooltip("The keyword to recognize")]
        public string keyword;
        [Tooltip("The UnityEvent invoked when the keyword is recognized by the KeywordRecognizer")]
        public UnityEvent onRecognized;
        [Tooltip("The minimum confidence level needed to invoke the event handler")]
        public ConfidenceLevel confidenceLevel = ConfidenceLevel.Low;

        public Keyword(string keyword, UnityEvent handler = null, ConfidenceLevel confidenceLevel = ConfidenceLevel.Low)
        {
            this.keyword = keyword;
        }
    }

    // This list is where the user actually inputs ther
    [Tooltip("The list of keywords and their associated event handlers")]
    public List<Keyword> keywordHandlers = new List<Keyword>
    {
        new KeywordHandler("My Keyword")
    };

    // Note: We would prefer to start with a dictionary and expose the mapping in the editor, but this is not supported by Unity
    private Dictionary<string, KeywordHandler> keywordHandlerMap;

    private KeywordRecognizer keywordRecognizer;

    void OnEnable()
    {
        CreateKeywordRecognizer();
        keywordRecognizer.Start();
    }

    void OnDisable()
    {
        keywordRecognizer.Stop();
    }

    private void CreateKeywordRecognizer()
    {
        // Stop the keyword recognizer if one is running
        if (keywordRecognizer != null)
            keywordRecognizer.Dispose();

        // Create a keyword handler map for eaay reference later
        keywordHandlerMap = keywordHandlers.ToDictionary(keywordHandler => keywordHandler.keyword, 
                                                         keywordHandler => keywordHandler);

        // Create a new keyword recognizer
        keywordRecognizer = new KeywordRecognizer(keywordHandlerMap.Keys.ToArray<string>());
        keywordRecognizer.OnPhraseRecognized += PhraseRecognized;
    }

    private void PhraseRecognized(PhraseRecognizedEventArgs args)
    {
        // Get the handler and invoke its event method if the confidence requirement is met
        KeywordHandler keywordHandler = keywordHandlerMap[args.text];
        if (args.confidence <= keywordHandler.confidenceLevel)
            keywordHandler.handler.Invoke();
    }

    /// <summary>
    /// Add a new keyword handler. If already registered the confidence and event handler will be replaced.
    /// </summary>
    /// <param name="keyword">The keyword to recognizer</param>
    /// <param name="unityEvent">A UnityEvent that is invoked when the keyword is recognized by the KeywordRecognizer</param>
    /// <param name="confidenceLevel">The minimum confidence level of the identified keyword required to invoke the event handler</param>
    public void AddKeywordHandler(string keyword, UnityEvent handler, ConfidenceLevel confidenceLevel = ConfidenceLevel.Low)
    {
        // Removes the old handler if present
        RemoveKeywordHandler(keyword);

        // Create and add a new handler
        KeywordHandler keywordHandler = new KeywordHandler(keyword, handler, confidenceLevel);
        keywordHandler.keyword = keyword;
        keywordHandler.handler = handler;
        keywordHandler.confidenceLevel = confidenceLevel;
        keywordHandlers.Add(keywordHandler);

        // Recreate the keyword recognizer
        CreateKeywordRecognizer();

        // Start it up if this component is enabled
        if (enabled)
            keywordRecognizer.Start();
    }

    /// <summary>
    /// Removes the handler for 'keyword'
    /// </summary>
    /// <param name="keyword">The keyword to remove</param>
    public void RemoveKeywordHandler(string keyword)
    {
        keywordHandlers = keywordHandlers.Where(keywordHandler => keywordHandler.keyword != keyword).ToList();
    }
}
