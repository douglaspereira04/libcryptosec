#include <libcryptosec/certificate/CertificateBuilder.h>
#include <libcryptosec/RSAKeyPair.h>
#include <fstream>
#include "gtest.h"
#include <iostream>
#include <sstream>

using std::endl;

class CertificateBuilderTest : public ::testing::Test {

protected:
	virtual void SetUp() {
		certBuilder = 0;
		req = 0;
	}

    virtual void TearDown() {
    	delete certBuilder;
        delete req;
    }

    RDNSequence getRdnAfterUpdateSubject(RDNSequence &rdnAlterSubject) {
    	std::string pem = "-----BEGIN CERTIFICATE REQUEST-----" "\n"
    			"MIIC9DCCAdwCAQAwga4xCzAJBgNVBAYTAkJSMRMwEQYDVQQIDApTb21lLVN0YXRl" "\n"
    			"MRMwEQYDVQQKDApJQ1AtQnJhc2lsMTQwMgYDVQQLDCtBdXRvcmlkYWRlIENlcnRp" "\n"
    			"ZmljYWRvcmEgUmFpeiBCcmFzaWxlaXJhIHYyMT8wPQYDVQQDDDZBdXRvcmlkYWRl" "\n"
    			"IENlcnRpZmljYWRvcmEgZGEgQ2FzYSBkYSBNb2VkYSBkbyBCcmFzaWwgdjMwggEi" "\n"
    			"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC/uaaTgYwZn7ljN+gZ8CaLQxaE" "\n"
    			"F/cUuidC48ms6QHTzRLhyQlTadBW/0hnVFHZ5yEXCNAmlOTLkFVroyeu6E8LK5Gx" "\n"
    			"R0rFP6kuhofjvEiJiUCGZdk2wjVl8S8FqqlO6kYriYqi2Mv0O3NRJ2PP+dLNO4ja" "\n"
    			"Xu/SzkpKBXuPwoOpTvmwOr/vyiivDfZE3TteMC1YmVO2vW5gP1jHQECKOSWbvZUo" "\n"
    			"SEXmkLr6+EzgmUFcabf2mhKyxaSAPKbAx3U5zNMwR0Sq+Vdgiv2nVCRQ/hiaZ9cc" "\n"
    			"YFBUCs9yqs4ffpzasZbwsdnxdRd6sNRpMlWS7hGfEuSWQxNGLGcwXWRWk4vhAgMB" "\n"
    			"AAGgADANBgkqhkiG9w0BAQUFAAOCAQEAPKWxdsutgYT7PXiCqWf8y31JK6Jl1Ify" "\n"
    			"iG6FeRTODLn+rv7Ehu3qpcZKbMUMfYrjWAi9i34TF5nkJTo26xlF0VzwO7MAsy51" "\n"
    			"cOXvEmMqVf9BUYjJ9Ig+JEAD7ucz0/zovX9wBwU+EhYwRvfRbXSf4INsoTmW6LBw" "\n"
    			"OXkmGSQANvvYr7gWDgbaW5D7p3mxLoN4YBdu8MBWFVJ3h3+BNowyqD2t+ye977zi" "\n"
    			"xFKAjfXIijHVfoOUwC/QRO57CBzB1TNTNxNPa+j2SYQKrImGX4nLkJE2WA8vwY37" "\n"
    			"+eX2YIRBXRS8KrwGWNo0CTiN1PDL2iceXJExrxKVjA51o9j9tVAnPw==" "\n"
    			"-----END CERTIFICATE REQUEST-----";
    	CertificateRequest certReq(pem);
    	certBuilder = new CertificateBuilder(certReq);

    	certBuilder->alterSubject(rdnAlterSubject);
    	RDNSequence newRdn = certBuilder->getSubject();

    	return newRdn;
    }

    RDNSequence buildRDNSubject(std::string country, std::string organization,
    		std::string oUnit, std::string common_name) {
    	RDNSequence rdnSubject;
		rdnSubject.addEntry(RDNSequence::COUNTRY, country);
		rdnSubject.addEntry(RDNSequence::ORGANIZATION, organization);
		rdnSubject.addEntry(RDNSequence::ORGANIZATION_UNIT, oUnit);
		rdnSubject.addEntry(RDNSequence::COMMON_NAME, common_name);

		return rdnSubject;
    }

    /*!
     * @brief Preenche as entradas de rdn.
     */
    void fillRDN() {
        RDNSequence tmp = RDNSequence();
        tmp.addEntry(RDNSequence::COUNTRY, "CO");
        tmp.addEntry(RDNSequence::STATE_OR_PROVINCE, "State");
        tmp.addEntry(RDNSequence::LOCALITY, "Locality");
        tmp.addEntry(RDNSequence::ORGANIZATION, "Organization");
        tmp.addEntry(RDNSequence::ORGANIZATION_UNIT, "Organization Unit");
        tmp.addEntry(RDNSequence::COMMON_NAME, "Common Name");
        rdn = tmp;
    }

    void modifyRDN() {
    	std::vector<std::pair<ObjectIdentifier, std::string> > entries = rdn.getEntries();
    	RDNSequence tmp;
    	tmp.addEntry(id2Type(entries[0].first.getNid()), "MD");    // precisa ter duas letras pois pode ser country name
    	for (unsigned int i = 1; i < entries.size(); i++) {
    		tmp.addEntry(id2Type(entries[i].first.getNid()), entries[i].second);
    	}
    	rdn = tmp;
    }

    int getReqCodification() {
    	X509_NAME* name = X509_get_subject_name(certBuilder->getX509());
    	for (int i = 0; i < X509_NAME_entry_count(name); i++) {
    		X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, i);
    		if (OBJ_obj2nid(entry->object) != NID_countryName) {
    			return entry->value->type;
    		}
    	}
    	return -1;
    }

    /*!
     * @brief Testa se a codificacao das entradas estao de acordo com o esperado.
     *
     * @param expectedCodification Codificacao esperada.
     */
    void testStringCodificaton(int expectedCodification) {
        X509_NAME* after = X509_get_subject_name(certBuilder->getX509());
        for (int i = 0; i < X509_NAME_entry_count(after); i++) {
            X509_NAME_ENTRY* entry = X509_NAME_get_entry(after, i);
            if (OBJ_obj2nid(entry->object) != NID_countryName) {
            	int codification = entry->value->type;
            	ASSERT_EQ(codification, expectedCodification);
            }
        }
    }

    /*!
     * @brief Testa se a codificacao do certificado esta de acordo com o esperado.
     *
     * @param expectedCodification Codificação esperada.
     * @param cert Certificado exportado.
     */
    void testStringCodificaton(int expectedCodification, Certificate* cert) {
    	X509_NAME* after = X509_get_subject_name(cert->getX509());
    	for (int i = 0; i < X509_NAME_entry_count(after); i++) {
    		X509_NAME_ENTRY* entry = X509_NAME_get_entry(after, i);
    		if (OBJ_obj2nid(entry->object) != NID_countryName) {
    			int codification = entry->value->type;
    			ASSERT_EQ(expectedCodification, codification);
    		}
    	}
    }

    /*!
     * @brief Testa se os valores das entradas sao mantidos apos a geracao do certificado.
     *
     * @param r RDNSequence o qual se deseja comparar com rdn interno.
     */
    void testStringValues(RDNSequence r) {
        vector<pair<ObjectIdentifier, string> > entries_before = r.getEntries();
        vector<pair<ObjectIdentifier, string> > entries_after = rdn.getEntries();
        for (unsigned int i = 0; i < entries_before.size(); i++) {
            if (entries_before[i].first.getNid() == entries_after[i].first.getNid()) {
            	string before = entries_before[i].second;
            	string after = entries_after[i].second;
            	ASSERT_EQ(before, after);
            }
        }
    }

    /*!
     * @brief Testa se os RDNs estão na ordem padrão OpenSSL.
     */
    void testRDNOrder() {
    	vector<pair<ObjectIdentifier, std::string> > entries = rdn.getEntries();
    	int previous_type = id2Type(entries[0].first.getNid());
    	for (unsigned int i = 1; i < entries.size(); i++) {
    		int current_type = id2Type(entries[i].first.getNid());
    		ASSERT_GE(current_type, previous_type);
    		previous_type  = current_type;
    	}
    }

    /*!
     * @brief Define a requisica a ser testada como uma requisicao com o DN totalmente preenchido com codificacao
     *        V_ASN1_PRINTABLESTRING.
     */
    void initializeCertRequestAndBuilder(int type) {
        std::stringstream stream;
    	switch(type) {
    	case FULL_PRINTABLE:
            stream << "-----BEGIN CERTIFICATE REQUEST-----" << endl
                   << "MIIDYTCCAkkCAQAwfDELMAkGA1UEBhMCUVExFjAUBgNVBAoTDW9yZyB5d3lyYSBB" << endl
                   << "QzExGDAWBgNVBAsTD3Vub3JnIHl3eXJhIEFDMTEZMBcGA1UECBMQZXN0YWRvIHl3" << endl
                   << "eXJhIEFDMTESMBAGA1UEAxMJeXd5cmEgQUMxMQwwCgYDVQQrEwNZV1kwggEiMA0G" << endl
                   << "CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCjNaz7BMH/QOpAjf85A1gu2MAiHNiK" << endl
                   << "Ne44BAwwWWz9tA3qyp543EF1ifj47zdL94rrP6+d/3YjaUNFPcaKf9cO+IFHv+OM" << endl
                   << "X5DL8bw4dX7kf1HAjmx4c0+h7weR2hhpDiPiit13GP5xjunBHppnoW1iS8sw2Mcw" << endl
                   << "m7iq6CqeNxGpsItJP2mvhPRxZvwDTo4X5BFyNwT33Z3eLZ6FBYWMUwvs44E2/Hwy" << endl
                   << "nMaCUyxuuGUtQ3vmRfwGZK8qIMuZ0q9ekPvrtOW5rsOc+hKDy4C38zMr7moOKkba" << endl
                   << "mHWLMKNKvT2+T7T82Hh0H/2IjtmQoDfZ+unmbCnmnicgqkmC6EV/23LtAgMBAAGg" << endl
                   << "gZ8wgZwGCSqGSIb3DQEJDjGBjjCBizAdBgNVHQ4EFgQUAievAyNbAkadZzZRAyn/" << endl
                   << "ZhvMFzcwKQYDVR0RBCIwIIIeY2FtcG8gcXVhbHF1ZXIgY29pc2EgeXd5cmEgQUMx" << endl
                   << "MA8GA1UdEwEB/wQFMAMBAf8wDwYDVR0PAQH/BAUDAwcHgDAdBgNVHSUEFjAUBggr" << endl
                   << "BgEFBQcDAQYIKwYBBQUHAwIwDQYJKoZIhvcNAQENBQADggEBAHGTUJZuOunsHq20" << endl
                   << "JkCz6rB36GhUiosFHgTzQrTfuU+th1us3gQTm76ZFA85nDKqcn7R1+QOPWDxF/1n" << endl
                   << "sgVljJqxPq23kd/P6dcKrsF7/bi/6DTbAQrIygHfIpjCRQWfbP/N7qg+cnZILXLI" << endl
                   << "slJZW7HHztvG6QicahLJm9SZttLhm8gcDOKMNbrrolGSalMrPUg4qGI/jzrLZ68E" << endl
                   << "nLTH+2ZKN8NdQpfTdgQ8BzcArYpXiYIe9VUUnFhFutWBh+cxcYXQuWRtbh+/AQiF" << endl
                   << "fQndDsfisQnMHy/F7zaKtuoIH46AcM52NkDxn0FXEQaUBefgvywhBD0biQvPPJQU" << endl
                   << "wfUDLbQ=" << endl
                   << "-----END CERTIFICATE REQUEST-----";
    		break;
    	case FULL_UTF8:
        	stream << "-----BEGIN CERTIFICATE REQUEST-----" << endl
        		   << "MIIDjzCCAncCAQAwgacxCzAJBgNVBAYTAkFBMRwwGgYDVQQKDBNvcmcgeXd5cmFB" << endl
    			   << "QzEtMi41LjRjMR4wHAYDVQQLDBV1bm9yZyB5d3lyYUFDMS0yLjUuNGMxHzAdBgNV" << endl
    			   << "BAgMFmVzdGFkbyB5d3lyYUFDMS0yLjUuNGMxGDAWBgNVBAMMD3l3eXJhQUMxLTIu" << endl
    			   << "NS40YzEfMB0GA1UEDAwWdGl0dWxvIHl3eXJhQUMxLTIuNS40YzCCASIwDQYJKoZI" << endl
    			   << "hvcNAQEBBQADggEPADCCAQoCggEBAJVY01W9ieCEfheLSrHcjxynj6fVwOyhGj1d" << endl
    			   << "4PwSqe0l8M4vYPMGRuC0NqsvOrps+McdFXjdBoElDEZ+7UrxFXEW3jyNPw30aei9" << endl
    			   << "3PtPWwS0UNj5ySs3nw3ybuZlmstFyeOldTbEFOctG/sVeHcz/pprVvqtCEXpLKtg" << endl
    			   << "vtDgzjd68RRpJVIWIQ46HuoZZTRQomMiHZdkKKALkS0eAI7cKn1fVoLze2Hi/xnV" << endl
    			   << "acPy6BN+nBhndUGye5KvmQUAI9hQk6UHORwLEuSW2tn8iWVopswmc1xMXez8sNPR" << endl
    			   << "d2xwNnfA6xBZ1IZ0ZegzPLgegKQYraU93RofJa1bIWAHG/gtdx8CAwEAAaCBoTCB" << endl
    			   << "ngYJKoZIhvcNAQkOMYGQMIGNMB0GA1UdDgQWBBSaXq9MlK7J+UHMhU84IwqvgUCY" << endl
    			   << "qDAXBgNVHREEEDAOggxhc2Rhc2RzYWRhc2QwDwYDVR0TAQH/BAUwAwEB/zAPBgNV" << endl
    			   << "HQ8BAf8EBQMDBz+AMDEGA1UdJQQqMCgGCCsGAQUFBwMDBggrBgEFBQcDBAYIKwYB" << endl
    			   << "BQUHAwgGCCsGAQUFBwMJMA0GCSqGSIb3DQEBDQUAA4IBAQBhtwGISNB+nwpEJdB3" << endl
    			   << "J1hYH28/61CpYhHGa5ysdCPaPpPw5P4+yrpC/iJFV4Lw/0pvauaTY8KVIZQnmLYq" << endl
    			   << "rDB3Rv1d1qEC1owv1FLocnXVTfpyqLHtTZkNJ7ApHzUhUl+YCW+/cuIU+B9RxDug" << endl
    			   << "H0511zsavlRO+9DkZYOC5hO/bDlWy1IdT51qepBWCRD2sHviMQRlzYrt/s2BpXKf" << endl
    			   << "bhPfBWLL0wvV18WA3JmUOnjOCMXpHni9qoKS36eZVUr1pLbvwEmh0OLtSu9hhQGu" << endl
    			   << "q3VVrMLsnvlcgBzTIZi4Nt52bqSHuYCrJh+pZdd3IPl5G0mra+HkuXnxKrbQM6YK" << endl
    			   << "3aUS" << endl
    			   << "-----END CERTIFICATE REQUEST-----";
    		break;
    	case INCOMPLETE_PRINTABLE:
        	stream << "-----BEGIN CERTIFICATE REQUEST-----" << endl
        		   << "MIIC+TCCAeECAQAwQzESMBAGA1UEBxMJeXd5cmEgQUMyMRIwEAYDVQQDEwl5d3ly" << endl
    			   << "YSBBQzIxGTAXBgNVBAwTEHRpdHVsbyB5d3lyYSBBQzIwggEiMA0GCSqGSIb3DQEB" << endl
    			   << "AQUAA4IBDwAwggEKAoIBAQDVh+Mi3eOz0YXK6J9hqCCwSLhAVpCHqxnGoq4g6bzL" << endl
    			   << "igClV5GbwIaKhMVuOS/0mdth+v4aBA1gVFMtpmR3xxFrDnaARjM5bwx1FKVyyZkF" << endl
    			   << "boNwaUGVWwPNraNNlnwMtL6oeksTDMSBRKTp8Jeu+sOPetL09ek4Ys29VGgRyu7i" << endl
    			   << "tE44fRiY0g+KxJfYN9DGPXv0dfJHhhu4D3UjvafAE0b8eiQT+4dlLw1euU1sJ8IF" << endl
    			   << "1y0109Jh5hexbLczhGuaV7bnG3xP+rrObiQ1iuQOdFMTgv1HTVnCmMM9Phh1lMXH" << endl
    			   << "5Sy8T1DZSfe0uekJpFuzbBTjzDQvb2ZYE7XM0/uoTWDDAgMBAAGgcTBvBgkqhkiG" << endl
    			   << "9w0BCQ4xYjBgMB0GA1UdDgQWBBRuVgfS7weLK4LybliWkb+q6LcVMjAPBgNVHRMB" << endl
    			   << "Af8EBTADAQH/MA8GA1UdDwEB/wQFAwMHH4AwHQYDVR0lBBYwFAYIKwYBBQUHAwEG" << endl
    			   << "CCsGAQUFBwMCMA0GCSqGSIb3DQEBDQUAA4IBAQCgEy2RmLok0IZizx9m++0v3YAn" << endl
    			   << "eOD2VogN2QcncEz92uLsorRgGe5uwqRMxeHcJoFVzPNvYphe0R6lV4mxjDCgUIwT" << endl
    			   << "hQl7GmCVEGMR+yCsgirYFHZz9jfuh7Q47ukSKo1sNrd50u8bFyUzu5CsnjEtJVE0" << endl
    			   << "2gJBCuiyXuYBg/L4eZJDoJwY/iKyQKhRd68BtEUXFr7wF0U0CkggPU38Kiy/VQLH" << endl
    			   << "XciyBd1S/BbTT9F8RW547rpeCF4oeqbN6kr2a+ykSIp3jLxz12vdXgGBVd+oBl6u" << endl
    			   << "H42k/Nd8kyfCQCuZY0+8fQZv9lHLDIeCKV5EBrbz93esyWHFMmePiRXCCi3v" << endl
    			   << "-----END CERTIFICATE REQUEST-----";
    		break;
    	case INCOMPLETE_UTF8:
        	stream << "-----BEGIN CERTIFICATE REQUEST-----" << endl
        		   << "MIIC7jCCAdYCAQAwPzEjMCEGA1UEBwwabG9jYWxpZGFkZSBZd3lyYUFDMi0yLjUu" << endl
    			   << "NGMxGDAWBgNVBAMMD1l3eXJhQUMyLTIuNS40YzCCASIwDQYJKoZIhvcNAQEBBQAD" << endl
    			   << "ggEPADCCAQoCggEBAL9rgsnYC+gfpoYdOTKqFn6JWsL56lrc0qPuXfx15OJ0JF1s" << endl
    			   << "tX3hMxkM+Jnq53kEOmVwKsVyCYvCBqzARFqzmSy2RtNr9UUlsQWDIcPel9c4Zzj4" << endl
    			   << "cufd2ve7ChAEzFTR4j+gLZAnx7J7UdrLSSToRIkQpclGjFy11a1ldj7EXfZjn7HY" << endl
    			   << "PDgYbS9b3GUp9zcJ8YkGMBiQJmCTVfsGL+81e/shxjnSI4AFc2FLKv6BgRH+g97c" << endl
    			   << "gXzn45FmacudY8T1gqt4/j5SlwXNXMmnIpG8FRgnEmd5DInT7sb9wVVf1Ei/YCRP" << endl
    			   << "vqCMAmPEx9sS5H+Smy8eI6CM64IFj2ElEqgbVk0CAwEAAaBqMGgGCSqGSIb3DQEJ" << endl
    			   << "DjFbMFkwHQYDVR0OBBYEFBfmUjxv/PjEd9kCwPZOeV2mjPXsMA8GA1UdEwEB/wQF" << endl
    			   << "MAMBAf8wJwYDVR0lBCAwHgYIKwYBBQUHAwEGCCsGAQUFBwMCBggrBgEFBQcDAzAN" << endl
    			   << "BgkqhkiG9w0BAQ0FAAOCAQEANczEwCLolws/rnLXFDJKgj3y4YmoH6L8BQ7tojwp" << endl
    			   << "FFQolVhkBXFu80ZY6OnH6WhR2Ux/1H4rp7UC/m/yNcKA54Jru0VGm40YcbfZo9BT" << endl
    			   << "gJoS0IZY9fjvalc4Wp7j2aeOAPoyPP75OrgZC3iGKxkZXe/0DmrmLgVPjUg3XtvE" << endl
    			   << "BkkFtOBZxvjrv2fMWSKTgb4GkcF2Jl7DDx2TcZBltjxetyUmjUojhwyoCZoAJT4t" << endl
    			   << "EyDVIOvHGRQYsl00eX+MWuqgzoosZkWc9LZTAasogMpeLQGG0016tfr4juPq2tOl" << endl
    			   << "FNH73IwYynRBrS/XwxG/WZDPdyqf563Xq/FTF/CfsmJzrw==" << endl
        		   << "-----END CERTIFICATE REQUEST-----";
    		break;
    	default:
    		FAIL();
    	}

    	std::string req_pem_encoded = stream.str();
        req = new CertificateRequest(req_pem_encoded);
	    certBuilder = new CertificateBuilder(*req);
        rdn = RDNSequence(certBuilder->getSubject().getX509Name());
    }

    /*!
     * @brief Converte um número representando um nid em um RDNSequence::EntryType.
     *
     * @param nid Nid OpenSSL.
     */
    RDNSequence::EntryType id2Type(int nid) {
    	switch (nid) {
    	case NID_countryName:            return RDNSequence::COUNTRY;
    	case NID_stateOrProvinceName:    return RDNSequence::STATE_OR_PROVINCE;
		case NID_localityName:           return RDNSequence::LOCALITY;
		case NID_organizationName:       return RDNSequence::ORGANIZATION;
		case NID_organizationalUnitName: return RDNSequence::ORGANIZATION_UNIT;
		case NID_commonName:             return RDNSequence::COMMON_NAME;
		default:                         return RDNSequence::UNKNOWN;
    	}
    }

    enum CertificateRequestType {
    	FULL_PRINTABLE,
		FULL_UTF8,
		INCOMPLETE_PRINTABLE,
		INCOMPLETE_UTF8
    };

    CertificateBuilder *certBuilder;
    CertificateRequest* req;  //!< CertificateRequest usado para aplicar a funcao testada.
    RDNSequence rdn; //!< RDNSequence no qual a funcao deve ser aplicada.

};

TEST_F(CertificateBuilderTest, AlterSubject_EmptyCommonName) {
	RDNSequence rdnAlterSubject = buildRDNSubject("BR", "ICP-Brasil",
			"Autoridade Certificadora Raiz Brasileira v2", "");
	RDNSequence rdnNewSubject = getRdnAfterUpdateSubject(rdnAlterSubject);

	std::string country = rdnNewSubject.getEntries(RDNSequence::COUNTRY).back();
	std::string organization = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION).back();
	std::string oUnit = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION_UNIT).back();

	EXPECT_STREQ(country.c_str(), "BR");
	EXPECT_STREQ(organization.c_str(), "ICP-Brasil");
	EXPECT_STREQ(oUnit.c_str(), "Autoridade Certificadora Raiz Brasileira v2");
	EXPECT_EQ(0, (int) rdnNewSubject.getEntries(RDNSequence::COMMON_NAME).size());
}

TEST_F(CertificateBuilderTest, AlterSubject_EmptyOrganizationUnit) {
	RDNSequence rdnAlterSubject = buildRDNSubject("BR", "ICP-Brasil",
			"", "Autoridade Certificadora da Casa da Moeda do Brasil v3");
	RDNSequence rdnNewSubject = getRdnAfterUpdateSubject(rdnAlterSubject);

	std::string country = rdnNewSubject.getEntries(RDNSequence::COUNTRY).back();
	std::string organization = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION).back();
	std::string commonName = rdnNewSubject.getEntries(RDNSequence::COMMON_NAME).back();

	EXPECT_STREQ(country.c_str(), "BR");
	EXPECT_STREQ(organization.c_str(), "ICP-Brasil");
	EXPECT_EQ(0, (int) rdnNewSubject.getEntries(RDNSequence::ORGANIZATION_UNIT).size());
	EXPECT_STREQ(commonName.c_str(), "Autoridade Certificadora da Casa da Moeda do Brasil v3");
}

TEST_F(CertificateBuilderTest, AlterSubject_EmptyOrganization) {
	RDNSequence rdnAlterSubject = buildRDNSubject("BR", "", "Autoridade Certificadora Raiz Brasileira v2",
			"Autoridade Certificadora da Casa da Moeda do Brasil v3");
	RDNSequence rdnNewSubject = getRdnAfterUpdateSubject(rdnAlterSubject);

	std::string country = rdnNewSubject.getEntries(RDNSequence::COUNTRY).back();
	std::string oUnit = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION_UNIT).back();
	std::string commonName = rdnNewSubject.getEntries(RDNSequence::COMMON_NAME).back();

	EXPECT_STREQ(country.c_str(), "BR");
	EXPECT_EQ(0, (int) rdnNewSubject.getEntries(RDNSequence::ORGANIZATION).size());
	EXPECT_STREQ(oUnit.c_str(), "Autoridade Certificadora Raiz Brasileira v2");
	EXPECT_STREQ(commonName.c_str(), "Autoridade Certificadora da Casa da Moeda do Brasil v3");
}

TEST_F(CertificateBuilderTest, AlterSubject_EmptyCountry) {
	RDNSequence rdnAlterSubject = buildRDNSubject("", "ICP-Brasil", "Autoridade Certificadora Raiz Brasileira v2",
			"Autoridade Certificadora da Casa da Moeda do Brasil v3");
	RDNSequence rdnNewSubject = getRdnAfterUpdateSubject(rdnAlterSubject);

	std::string organization = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION).back();
	std::string oUnit = rdnNewSubject.getEntries(RDNSequence::ORGANIZATION_UNIT).back();
	std::string commonName = rdnNewSubject.getEntries(RDNSequence::COMMON_NAME).back();

	EXPECT_EQ(0, (int) rdnNewSubject.getEntries(RDNSequence::COUNTRY).size());
	EXPECT_STREQ(organization.c_str(), "ICP-Brasil");
	EXPECT_STREQ(oUnit.c_str(), "Autoridade Certificadora Raiz Brasileira v2");
	EXPECT_STREQ(commonName.c_str(), "Autoridade Certificadora da Casa da Moeda do Brasil v3");
}

/*!
 * @brief Testa se o certificado mantem a formatacao antes de ser emitido.
 */
TEST_F(CertificateBuilderTest, EncodingTest_PrintableCodification) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_PRINTABLESTRING);
}

TEST_F(CertificateBuilderTest, EncodingTest_UTF8Codification) {
	initializeCertRequestAndBuilder(FULL_UTF8);
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_UTF8STRING);
}

/*!
 * @brief Testa se o certificado mantem a formatacao antes de ser emitido, com um campo adicionado durante a emissao.
 */
TEST_F(CertificateBuilderTest, EncodingTest_StringValues) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
    certBuilder->alterSubject(rdn);
    testStringValues(req->getSubject());
}

/*!
 * @brief Testa se o certificado mantem a formatacao antes de ser emitido, com um campo adicionado durante a emissao.
 */
TEST_F(CertificateBuilderTest, EncodingTest_AddedFieldPrintableCodification) {
	initializeCertRequestAndBuilder(INCOMPLETE_PRINTABLE);
    rdn.addEntry(RDNSequence::ORGANIZATION_UNIT, "OUnitName");
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_PRINTABLESTRING);
}

TEST_F(CertificateBuilderTest, EncodingTest_AddedFieldUTF8Codification) {
	initializeCertRequestAndBuilder(INCOMPLETE_UTF8);
    rdn.addEntry(RDNSequence::ORGANIZATION_UNIT, "OUnitName");
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_UTF8STRING);
}

TEST_F(CertificateBuilderTest, EncodingTest_ModifiedFieldPrintableCodification) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
    modifyRDN();
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_PRINTABLESTRING);
}

TEST_F(CertificateBuilderTest, EncodingTest_ModifiedFieldUTF8Codification) {
	initializeCertRequestAndBuilder(FULL_UTF8);
    modifyRDN();
    certBuilder->alterSubject(rdn);
    testStringCodificaton(V_ASN1_UTF8STRING);
}

TEST_F(CertificateBuilderTest, EncodingTest_ModifiedFieldStringValues) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
    modifyRDN();
    RDNSequence modified = rdn;
    certBuilder->alterSubject(rdn);
    testStringValues(modified);
}

TEST_F(CertificateBuilderTest, EncodingTest_ExportedCertificatePrintableCodification) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
	certBuilder->alterSubject(rdn);
	RSAKeyPair key = RSAKeyPair(4096);
	Certificate* cert = certBuilder->sign(*key.getPrivateKey(), MessageDigest::SHA512);
	testStringCodificaton(V_ASN1_PRINTABLESTRING, cert);
}

TEST_F(CertificateBuilderTest, EncodingTest_ExportedCertificateUTF8Codification) {
	initializeCertRequestAndBuilder(FULL_UTF8);
	certBuilder->alterSubject(rdn);
	RSAKeyPair key = RSAKeyPair(4096);
	Certificate* cert = certBuilder->sign(*key.getPrivateKey(), MessageDigest::SHA512);
	testStringCodificaton(V_ASN1_UTF8STRING, cert);
}

TEST_F(CertificateBuilderTest, EncodingTest_ExportedCertificateStringValues) {
	initializeCertRequestAndBuilder(FULL_PRINTABLE);
	certBuilder->alterSubject(rdn);
	RSAKeyPair key = RSAKeyPair(4096);
	Certificate* cert = certBuilder->sign(*key.getPrivateKey(), MessageDigest::SHA512);
	testStringValues(cert->getSubject());
}

TEST_F(CertificateBuilderTest, EncodingTest_NewRequestDefaultCodification) {
	fillRDN();
	req = new CertificateRequest();
	req->setSubject(rdn);
	certBuilder = new CertificateBuilder(*req);
	int expectedCodification = getReqCodification();
	certBuilder->alterSubject(rdn);
	testStringCodificaton(expectedCodification);
}
